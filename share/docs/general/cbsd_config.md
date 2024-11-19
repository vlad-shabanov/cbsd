# CBSD Config/Customizing

The CBSD is a highly customized system, where different behavior or integration with other systems is achieved through configuration files.

The basic hierarchy of the working directory is:
```
   .
   .ssh/         - The directory stores the personal (generated by `cbsd initenv`) private key of the node. Can be used when adding nodes via SSH, as well as when using cloud VMs
   .rssh/        - The directory stores the peronal private key of the remote nodes.
   basejail/     - The directory stores "rootfs"/RO data for containers.
   etc/          - The directory stores CBSD (and modules) configuration files.
   export/       - By default, virtual environments are exported to this directory.
   import/       - By default, virtual environments are imported from this directory.
   jails/        - Mount points for containers ( "rootfs" basejail + container data overlay.
   jails-data/   - This is where containers and virtual machines data is stored (can be a symbolic link to a ZFS volume).
   jails-fstab/  - FSTAB files for containers.
   jails-system/ - CBSD-specific per-VM/container settings.
   var/db        - Main SQLite3 CBSD database.
```

## Configuration

CBSD has a set of default profiles and parameters, which are described in the configuration files in the directory *~cbsd/etc/defaults/*.

YOU SHOULD NOT CHANGE these files, as all your changes will be destroyed when CBSD is reinitialized ( `cbsd initenv` ).

To override or add your own settings, use the *~cbsd/etc/* directory where you can place configuration files of the same name.

### Colors

For example, 'NOCOLOR=1' environment variable turns off color in CBSD output, e.g.:
```
env NOCOLOR=1 cbsd jls
```

You can adjust this setting via the configuration file and change a number of default settings - see *~cbsd/etc/defaults/global.conf*  as an example 
and overwrite the setting through a similar file at a higher level:
```
echo NOCOLOR=1 >> ~cbsd/etc/global.conf
```

Perhaps you want to change default ( *~cbsd/etc/defaults/color.conf* ) or write your own color scheme? For example, redefine ( *~cbsd/etc/defaults/color-white.conf* this is just an example for alternative scheme ) the colors:

```
cp -a ~cbsd/etc/defaults/color-white.conf ~cbsd/etc/color.conf
```

## Profiles

### Profile for jail settings

**Description**

For example, you do not work with a huge number of nodes and environments, when such tools as Puppet, Ansible, SaltStack, Chef or your own implementation for generating .jconf file for jcreate can be useful.
However, you often have to create environments and at the same time do not accept a number of default options. In this case we recommend that you use profiles to override the default values, or create your own profile

Take a look on content of _$workdir/etc/defaults/jail-freebsd-default.conf_ file. It represents the settings that are used in jconstruct-tui by default.

Let's say you want to create a container always in **baserw=1** mode (instead of baserw=0 by default), on interface **lo0** (instead of **auto**, which selects the interface depending on the subnet jail) and
you prefer a name **cell** instead of **jail** wih domain **example.com** and at the same time, container must be launched instantly after creation ( runasap="1" ) ( do not confuse with astart="1" for lauch on boot)

To do this, create in the directory (or copy from _$workdir/etc/defaults_) _$workdir/etc/_ file with same name, in which we can reassign the settings:

```
	echo 'baserw="1"' > ~cbsd/etc/jail-freebsd-default.conf
	echo 'interface="lo0"' >> ~cbsd/etc/jail-freebsd-default.conf
	echo 'default_jailname="cell"' >> ~cbsd/etc/jail-freebsd-default.conf
	echo 'default_domain="example.com"' >> ~cbsd/etc/jail-freebsd-default.conf
	echo 'runasap="1"' >> ~cbsd/etc/jail-freebsd-default.conf
```

So we got file _$workdir/etc/jail-freebsd-default.conf_ with follow content:

```
	baserw="1"
	interface="lo0"
	default_jailname="cell"
	default_domain="example.com"
	runasap="1"
```

It's all! Now we can run **cbsd jconstruct-tui**, where you need to do even fewer settings to create an environment!

You may want to create several profiles of your own. To do this, also create the files in the directory _~cbsd/etc/_ by using in the name of file prefix: _jail-freebsd- **YOUR\_PROFILE**.conf_

In this case, as the **\_invariant\_** parameter within each profile, there must be a name for this profile in the variable _jail\_profile="default"_ and **jail\_active** set to 1 (active profile).
For example, let's create two profiles: **baserw** and **lo0**:

```
	echo 'jail_profile="baserw"' > ~cbsd/etc/jail-freebsd-baserw.conf
	echo 'jail_active="1"' >> ~cbsd/etc/jail-freebsd-baserw.conf
	echo 'baserw="1"' >> ~cbsd/etc/jail-freebsd-default.conf
```

```
	echo 'jail_profile="lo"' > ~cbsd/etc/jail-freebsd-lo0.conf
	echo 'jail_active="1"' >> ~cbsd/etc/jail-freebsd-lo0.conf
	echo 'interface="lo0"' >> ~cbsd/etc/jail-freebsd-lo0.conf
```

Now you will see the profiles in the choice of possible options for the 'profile' menu at startup of **cbsd jconstruct-tui**

Perhaps you want the profile lo0 to be the default, since you use it most often. You can override in _$workdir/etc/jail-freebsd-default.conf_ profile by default via params **default\_profile**:

```
	% echo 'default_profile="lo"' > ~cbsd/etc/jail-freebsd-default.conf

```

The contents of the file _~cbsd/etc/jail-freebsd-default.conf_ will be:


```
	default_profile="lo"
```

In this case, when you execute **cbsd jconstruct-tui**, you do not even need to choose a profile. There are very few actions to run a new environment.

#### Profiles for jail content

You can create your own structure (skel directory) for a container with any arbitrary files and specify this directory as `jailskeldir=`

As existing examples:
```
ls ~cbsd/share/*-jail-*-skel
```

Take a look on ~cbsd/share/FreeBSD-jail-puppet-skel as example of alternative jailskeldir= for "puppet" profile (see below)

#### Profiles for jconstruct-tui

If you want to manually create these jails, choosing when it should be necessary skel-directories or the default settings, create the directory $workdir/etc file named jail-freebsd- **XXXX**.conf with minimum content:


```
jail_profile="XXX"
```

where XXX - name of the profile. Profile **vnet** in CBSD is created as an example, you see it in _$workdir/etc/defaults/jail-freebsd-vnet.conf_

In turn, it overwrites the default values from the profile defaults: _jail-freebsd-default.conf_

If you rarely use the default profile and want to CBSD default offered at your profile, change the value default\_profile putting it in the name of your profile:

```
echo 'default_profile="XXX"' > ~cbsd/etc/jail-freebsd-default.conf
```

As existing examples, take a look in "puppet" profile: ~cbsd/etc/defaults/jail-freebsd-puppet.conf


### Profiles for VM settings

Similar to containers, you can override and fix some default parameters for `bhyve`, `QEMU`, `XEN`... virtual machines. 
For example, you want to use bind VNC on ALL addresses instead of '127.0.0.1' and change default VNC password from 'cbsd' to 'supersecret' ?

for bhyve:
```
echo 'bhyve_vnc_tcp_bind="0.0.0.0"' >> ~cbsd/etc/bhyve-default-default.conf
echo 'default_vnc_password="supersecret"' >> ~cbsd/etc/bhyve-default-default.conf
```

for QEMU:
```
echo 'qemu_vnc_tcp_bind="0.0.0.0"' >> ~cbsd/etc/qemu-default-default.conf
echo 'default_vnc_password="supersecret"' >> ~cbsd/etc/qemu-default-default.conf
```

for XEN:
```
echo 'xen_vnc_tcp_bind="0.0.0.0"' >> ~cbsd/etc/xen-default-default.conf
echo 'default_vnc_password="supersecret"' >> ~cbsd/etc/xen-default-default.conf
```

(Also global ~cbsd/etc/vnc.conf can be used)

Want to change default 'interface', and enable auto-start by default ?

for bhyve:
```
echo "interface=bridge10" >> ~cbsd/etc/bhyve-default-default.conf
echo "astart=1" >> ~cbsd/etc/bhyve-default-default.conf
```

for QEMU:
```
echo "interface=bridge10" >> ~cbsd/etc/qemu-default-default.conf
echo "astart=1" >> ~cbsd/etc/qemu-default-default.conf
```

for XEN:
```
echo "interface=bridge10" >> ~cbsd/etc/xen-default-default.conf
echo "astart=1" >> ~cbsd/etc/xen-default-default.conf
```

And so on. Examine the corresponding configuration files in the *~cbsd/etc/defaults/* directory.
