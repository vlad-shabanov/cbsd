## [Using CBSDfile]()

You can use CBSDfile to create and remove **CBSD** environments by single command. If you are looking for Vagrant for jail or bhyve, you may like to enjoy a native FreeBSD/CBSD mechanism.

We didn't reinvent the wheel with the CBSDfile format, and we're not trying to replicate **Dockerfile-like** syntax: Docker (and Dockerfile) are designed with Linux in mind, 
and over 98 percent of the instructions you're used to simply won't work, which can be confusing.
Instead we use the shell script form familiar to most: we describe the configuration of your application in a function named **jail\_xxx** or **bhyve\_yyy**. One function is one environment. You can use any parameters from jcreate/jconstruct-tui/jconfig or bcreate/bconstruct-tui/bconfig commands.

For example, creating a CentOS7 virtual machine through bcreate:

```
cbsd bcreate jname=c1 vm_ram=4g vm_cpus=2 vm_os_type=linux vm_os_profile="cloud-CentOS-7-x86_64" ci_ip4_addr=10.0.0.110 ip4_addr=10.0.0.110 runasap=1 imgsize=6g
```

In **CBSDfile** format, it can be written as follows:

```
bhyve_c1()
{
	vm_ram="4g"
	vm_cpus="2"
	imgsize="6g"
	vm_os_type=linux
	vm_os_profile="cloud-CentOS-7-x86_64"
	runasap=1
	ssh_wait=1
}
```

Also, using a regular shell syntax to describe the environment, unlike **Dockerfile-like** files, we can use normal **sh** constructs and use different conditions. For example, directories with a CBSDfile can be shared by several independent nodes, but depending on a certain condition (for example, **by hostname**), the environments will use different IP addresses:

```
jail_nginx()
{
	case "${nodename}" in
		home.my.domain)
			ip4_addr="93.100.25.10,2a05:3580:d811:800::2"
			;;
		srv-01.convectix.com)
			# for srv-01.convectix.com
			ip4_addr="142.132.155.251,2a01:4f8:261:2b85::1,10.0.100.1"
			;;
		*)
			echo "Unkown nodename: $nodename, no ip4_addr profile/settings"
			ip4_addr="DHCP"
			;;
	esac

	ver="native"

	interface=0		# with interface=0, IP address 
					#  should already be initialized, e.g via rc.conf
	host_hostname="${jname}.convectix.com"
	astart=1
	pkg_bootstrap=1
	pkglist="www/nginx"
}
```

We will execute the **cbsd up** command and waiting for the end, you can immediately go to the virtual machine through **cbsd blogin**

If you need to perform a number of actions when creating an environment, use **postcreate\_XXX** functions.

Please note that command like: blogin,jlogin,bexec,jexec,bscp,jscp,bdestroy,jdestroy,bstop/jstop and so on, also support CBSDfile and if you execute the command:

```
cbsd blogin
```

in the directory where **CBSDfile** is located, **CBSD** will use environment from **CBSDfile**, even when **CBSDfile** uses API (see below).

## [CBSDfile + API (private cloud)]()

**CBSDfile** can work not only locally, but also with a remote API, which can be any number of nodes. By adding only two parameters to the file (**CLOUD\_URL** + **CLOUD\_KEY**), you will force **CBSDfile** to work with your API service, deploying your environments in your own private cloud! In this case, the local copy of the **CBSD** works as a thin client. Please read for details: [API Module: Private cloud via API](/en/cbsd_api_ssi.html)

The main differences of the **CBSDfile** for the API versus local are that instead of **vm\_os\_type** and **vm\_os\_profile** for bhyve you use a brief alias: **img=**, which is adjusted name to profile mapping on the server side, and for jail the **imgsize=** parameter is required that it will set a quota for a jail file system. And also, network settings (IP address, gateway, masks) with Cloud/API do not play roles, because the IP addresses controlled by server and you cannot ask any address.

## [Examples]()

It is best to get acquainted with Cbsdfile, looking at work examples. For example, look at CBSDfile for the deploy of the Gitlab service: <https://github.com/cbsd/cbsdfile-recipes/tree/master/jail/gitlab>

In **jail\_** or **bhyve\_** functions there is only a description of the environmental parameters. If you need when you turn around the environment to perform a number of actions in it, use the **postcreate\_** functions. This is an ordinary shell function in which you can use **CBSD** commands, where the **jname=** and **cbsd** prefix can be omitted. For example, if in the command line for port forwarding you write:

```
cbsd expose jname=jail2 in=2233 out=22
```

, then in the **CBSDfile** full example will look like:

```
jail_jail2()
{
	ver="native"			# Use the same version for FreeBSD base for jail as your host. You can specify fixed: ver=12.2 or ver=13.0 and so on..
	ip4_addr="DHCP,DHCPv6"		# You need correct configuration of nodeippool and nodeip6pool via 'cbsd initenv-tui`
	runasap=1			# Run Jail immediately upon creating

	allow_raw_sockets=1		# allow ICMP/ping in jail

	# turn off unnecessary services
	sysrc="syslogd_enable=NO cron_enable=NO sendmail_enable=NO sendmail_submit_enable=NO sendmail_outbound_enable=NO sendmail_msp_queue_enable=NO clear_tmp_enable=YES"
}

postcreate_jail2()
{
	expose in=2233 out=22
}
```

You may want to issue IP addresses not from the global pool, for this you can call cbsd 'dhcpd' script with alternative networks or IP address range, for example:

```
jail_jail2()
{
	ip4_addr=$( dhcpd ip4pool=192.168.0.5-10 )		# Will give an address from the range: 192.168.5-10
}
```

or list of IP addresses (or networks):

```
jail_jail2()
{
	ip4_addr=$( dhcpd ip4pool=10.0.0.2,10.0.0.3,10.0.0.4,10.0.0.190-200,10.1.0.0/24 )
}
```

If you plan to use the **CBSDfile** with API, when copying files from the host, please prefer **jscp** (for jail) and **bscp** (for bhyve), instead of a regular **cp**, since jscp/bscp scripts supports for CBSDfile/API. In this case, for local operations and cloud, your **CBSDfile** will be the same. For example:

```
jail_jail2()
{
	ip4_addr="DHCP					# You need correct configuration of nodeippool via 'cbsd initenv-tui`
	runasap=1				# Run Jail immediately upon creating
}

postcreate_jail2()
{
	cp /COPYRIGHT ${data}/root			# This command copies / Copyright inside the container in the / root directory only locally and will not work when Cloud API.
									# Do not use it. Instead, write:
	jscp /COPYRIGHT ${jname}/root/			# This command will work equally with a local deployment and Cloud/API
}
```

Another way to customize jail by static files: 'skel' directory. Just put files into skel/ dir hier to overwrite jail content, e.g.: [how to drop /usr/local/etc/smb4.conf and /usr/local/etc/pkg/repos/](https://github.com/cbsd/cbsdfile-recipes/tree/master/jail/sambashare/skel) files in Samba jail

.

Also, the parameters that you describe in **jail\_** and **bhyve\_** functions can be specified in the command line. What can be convenient if you use one **CBSDfile** to generate the several image of different versions, for example, one source:

```
jail_mc()
{
	ip4_addr="DHCP"				# You need correct configuration of nodeippool via 'cbsd initenv-tui`
	pkglist="shells/bash misc/mc"			# install bash and mc packages
	runasap=1				# Run Jail immediately upon creating
}
```

By running the \`cbsd up ver=14.2\` or \`cbsd up ver=14.1\`, you will receive a container with mc/bash for FreeBSD 14.2 or 14.1, without having to edit **CBSDfile**.

If you are using **CBSDfile** with the Cloud/API and when creating an environment, you perform a number of actions with it, make sure that you have **ssh\_wait=1** parameter. This will make 'cbsd up' script wait for access via SSH before the **postcreate\_XXX** function is performed:

```
jail_jail2()
{
	pkglist="puppet7"				# install puppet agent
	ssh_wait=1
}

postcreate_jail2()
{
	jscp /path/to/config/puppet.conf ${jname}:/usr/local/etc/puppet/		# copy puppet.conf, in which your Puppet Server is specified
	jexec /usr/local/bin/puppet agent -t						# apply configuration via Puppet
}
```

In this example, you can create environments that automatically be connected to your configuration via PuppetServer and further control you can be made via Puppet.

If you need to perform a sequence of actions via **jexec/bexec**, you can use the **HEREROC** design, instead of a large number of jexec/bexec calls, for example:

```
# for jail:
jail_jail2()
{
	pkglist="puppet7"				# install puppet agent
	ssh_wait=1					# Waiting for the accessibility of the environment before performing postcreate operations (when API used)
}

postcreate_jail2()
{
	jscp /path/to/config/puppet.conf ${jname}:/usr/local/etc/puppet/		# copy puppet.conf, in which your Puppet Server is specified
	# and perform all subsequent commands in the created environment:
	jexec <<EOF
export PATH="/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin"
whoami
date
hostname
pwd
EOF
}

# for bhyve:
bhyve_vm1()
{
	vm_ram="4g"
	vm_cpus="2"
	imgsize="6g"
	vm_os_type=linux
	vm_os_profile="cloud-CentOS-7-x86_64"
	runasap=1
	ssh_wait=1
}

postcreate_vm1()
{
	# and perform all subsequent commands in the created environment:
	bexec <<EOF
# All these commands are performed in a virtual machine:
export PATH="/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin"
sudo yum update -y                                                                 #
sudo yum -y install https://yum.puppet.com/puppet6-release-el-7.noarch.rpm         #    puppet package
sudo yum install -y puppet-agent                                                   #
sudo yum clean -y all                                                              #
EOF
}
```
A few more examples:
Example for jail. Create the requirents.txt file in the directory - it will contain a list of PIP modules for Python, with content:

```
requests
```

Create **CBSDfile**:

```
jail_pyjail1()
{
	ip4_addr="DHCP"
	host_hostname="${jname}.example.com"
	pkg_bootstrap="1"
	runasap=1
	pkglist="www/nginx python311 security/sudo security/ca_root_nss devel/py-pip devel/py-websockets"
}

postcreate_pyjail1()
{
	jscp requirements.txt ${jname}:/tmp/requirements.txt
	jexec pip install -r /tmp/requirements.txt
	jexec rm /tmp/requirements.txt
}
```

create and run pyjail1:
```
cbsd up
```

or:

```
cbsd up pyjail1
```

login into pyjail1:

```
cbsd jlogin
```

destroy pyjail1:

```
cbsd destroy
```

or:

```
cbsd destroy pyjail1
```

Other special "reserved" functions that can be used in CBSDfile:

1) You can take the same parameters from all environments and describe them only once via 'global()' func:
```
global()
{
	allow_raw_sockets=1
	host_hostname="${jname}.my.domain"
} 
```

2) you can perform some operations before or after creating the environment via 'preup_${jname}' or postup_${jname}';

3) you can perform some operation before and after creating the environment via 'predestroy_${jname}' and/or 'postdestroy_${jname}';

4) you can perform some operations before or after work of CBSDfile via 'preup()' and 'postup()';


Yet another CBSDfile sample [CBSDfile](https://github.com/cbsd/cbsd/blob/develop/share/examples/cbsdfile/CBSDfile)

Look at the demo as it may look for hybrid environments with the creation of Jail and Bhyve at the same time: 

![cbsdfile demo: jails](https://convectix.com/gif/cbsdfile-jail0.gif)

[CBSDfile: jails + bhyve demo](https://convectix.com/gif/cbsdfile.gif)

