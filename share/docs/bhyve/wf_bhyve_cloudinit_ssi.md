# bhyve and cloud-init with CBSD

## Commands: bconfig, bconstruct-tui, cloudinit

```
	% cbsd bconstruct-tui
	% cbsd cloudinit
	% cbsd bcreate
```

**Description**:

FYI: Cloud-init demo was demonstrated in:


- [Bhyve unattended installation with CBSD: PXE and cloud-init](https://youtu.be/QK9eSxrs3eg)
- [NetBSD cloud image for bhyve demo](https://youtu.be/3F_lh8s51_4)
- [Microsoft Windows Server 2016 cloud image via CBSD/cloudbase demo](https://youtu.be/PDkxrHb5Xt8)

Currently, the use of [cloud-init](https://cloud-init.io/) is a de facto method for rapidly deploying virtual instances in various
cloud environments (OpenNebula, OpenStack, Amazon AWS, Microsoft Azure, Digital Ocean ..)

Starting from version 12.0.6 **CBSD** supports the configuration of cloud images using the cloud-init function.

How it works:

Currently, **CBSD** is able to configure the instans of cloud-init using the **NoCloud data source method**.
This means that the virtual machine receives all the settings through the file system fat32/msdos\_fs or cd9660 of the connected local media.
The task of **CBSD** is to generate and connect the image to the virtual machine at the time of its launch.

As a sign that **CBSD** should activate cloud-init functions, is the fact that there is a cloud-init directory in the
system directory of a individual virtual machine: _${jailsysdir}/${jname}/cloud-init/_.
In addition to the sign of activation of cloud-init, this directory acts as a repository of configuration in the format and hierarchy of cloud-init,
which will be provided to the virtual machine. That is, if the **CBSD** working directory (cbsd\_workdir) is initialized in the
_/usr/jails_ directory, for the virtual machine named **vm1** the enable flag and parameters for configuring cloud-init should be
in _/usr/jails/jails-system/vm1/cloud-init_ directory.

For configuration format and configuration options for cloud-init, refer to the relevant [official project](https://cloudinit.readthedocs.io/en/latest/) information.

In addition, the **CBSD** distribution includes an example of a simple configuration that you can view in the
_/usr/local/cbsd/share/examples/cloud-init_ directory and use as a starting point for creating your own cloud installations.

Attention! Despite the [lack of binding to ZFS](http://www.bsdstore.ru/en/articles/cbsd_with_dfs.html),
installation using ZFS is recommended for use with cloud-init. In this case, **CBSD** uses COW technology in the form of a
zfs clone to create a virtual machine based on the cloud image. Otherwise, **CBSD** each time will be forced to perform a lengthy operation with a
standard copy of the cloud image on the virtual machine disk.
However, it is still much more efficient than installing via ISO using the installer every time.

Attention! In some cases, you may need a runtime configuration, for example, when using network-config version 1.
Unlike version 2, where you can use the match parameter with wildcard as the network interface name, the first version requires a strong interface name.
Which may vary depending on the numbering of the PCI bus. In this case, you may need the opportunity
[pre/post start/stop hooks](http://www.bsdstore.ru/en/13.0.x/wf_jconfig_ssi.html#execscript) in **CBSD**,
which helps you create dynamic configurations for cloud-init.

In addition, an assistant for cloud-init was added to the **CBSD** virtual machines configurator via bconstruct-tui,
which implements the minimum required configuration to get a running virtual machine from the cloud image.
For this you can use several pre-configured profiles with the **cloud-** prefix.

The number of profiles will increase over time. In addition, you can independently create and send a profile through a public GitHub repository:
[https://github.com/cbsd/cbsd-vmprofile](https://github.com/cbsd/cbsd-vmprofiles).
These are the profiles that the **CBSD** uses.


In addition, if you notice that image acquisition speed is low ( **CBSD** uses its own mirrors to duplicate images referenced by **CBSD** profiles),
and you have a desire to help the project, please read the information on how to raise your own mirror: [fetch\_iso](http://www.bsdstore.ru/en/cbsd_fetch.html#cbsd_partners).
You can send us a link to your mirror (or add it yourself via [https://github.com/cbsd/cbsd-vmprofile](https://github.com/cbsd/cbsd-vmprofiles),
and thereby improve the quality for your country/region.

Note: In **CBSD** version 12.0.8, parameter **ci\_user\_pubkey\_** user can accept not only ssh pubkey itself, but also the path to authorized\_keys.
In addition, if this parameter is set to **.ssh/authorized\_keys** (value by default for 12.0.8+), this means that your node's ssh key will be used. ( ~cbsd/.ssh ).
Pay attention to how looks [blogin.conf](https://github.com/cbsd/cbsd/blob/v12.0.8/etc/defaults/blogin.conf#L21) in 12.0.8: if the virtual machine is created using cloud-init,
the "cbsd blogin" command will use the custom login command using the node key and the user specified by you as **ci\_user\_add**.Thus, by running a virtual machine from cloud-init,
you can immediately access it via ssh using the command: **cbsd blogin**.

## bcreate from args

If you don’t like the dialog boxes, you can start the virtual machine from the cloud image as quickly as possible using bcreate by specifying the appropriate configuration parameters, for example:

```
cbsd bcreate jname=centos1 imgsize=20g vm_cpus=1 vm_ram=4g vm_os_type=linux vm_os_profile=cloud-CentOS-7-x86_64 ci_jname=centos1 ci_fqdn=centos1.my.domain ci_ip4_addr=DHCP ci_gw4=10.0.0.1 runasap=1
cbsd bcreate jname=centos2 imgsize=40g vm_cpus=4 vm_ram=8g vm_os_type=linux vm_os_profile=cloud-CentOS-8-x86_64 ci_jname=centos2 ci_fqdn=centos2.my.domain ci_ip4_addr=192.168.0.10/24 ci_gw4=192.168.0.1 runasap=1
```

pay attention to **ci\_ip4\_addr** and **ci\_gw4** \- when using the DHCP value, make sure that you select the correct nodeippool range of addresses, and also that ci\_gw4 points to the working IP of the gateway in this subnet.

## Usage example

Profiles of cloud images are in the vm\_os\_profile menu. Select this item in the main menu:

![](http://www.bsdstore.ru/img/cloudinit1.png)

At the bottom of the list, you will see an area with Cloud images,
if these profiles are created in **CBSD** for the selected OS family:

![](http://www.bsdstore.ru/img/cloudinit2.png)

Next, configure the network settings, user and public key of the guest machine:

![](http://www.bsdstore.ru/img/cloudinit3.png)

![](http://www.bsdstore.ru/img/cloudinit4.png)

Further configuration and launch of a cloud-based virtual machine is no different from the main method. Good luck!

