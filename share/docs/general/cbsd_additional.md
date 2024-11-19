# What you need to know about **CBSD**

## General information

**CBSD** is an additional layer of abstraction for the
[jail(8)](http://www.freebsd.org/cgi/man.cgi?query=jail&sektion=8)
framework, [bhyve hypervisor](http://www.freebsd.org/cgi/man.cgi?query=bhyve&sektion=8), [XEN project hypervisor](http://www.xenproject.org/) and some components of [FreeBSD OS](https://www.freebsd.org/).

The additional functionality **CBSD** provides uses the
following;


- vnet (VIMAGE)
- zfs
- racct/rctl
- ipfw
- pf/ipfw/ipfilter
- carp
- hastd
- vale
- vxlan

Although **CBSD** aims to be the most user-friendly application (for example, using bsdconfig-style dialogs),
the platform is evolving as an embedded virtual environment management system that you can use at the lowest level to create your own cloud infrastructure.

In other words, you can work directly with the **CBSD** as an end user interactively.
Or, you can use the **CBSD** as an internal core, interacting with it through your own application of a higher level.

**CBSD** assumes the use of multiple servers (cluster), but it can work equally well in a standalone version on your laptop.

While many of these subsystems are not directly related to
**jails** or **vm hypervisor**, **CBSD** uses these components to
provide system administrators a more advanced, integrated system in which
to implement solutions for issues faced in today's envirnonment.


This page will provide information to help system administrators familiarize
themselves with CBSD. While this page is not intended to be a comprehensive,
all encompassing how-to, it will provide details about where files are
stored, and how to use **CBSD** to manage and interact with
the virtual environment.


The information provided here assumes a basic understanding of jails,
how they are used, and how they are managed in FreeBSD. If you plan to work with containers, the official
documentation about jails is a highly recommended starting point, and can be
found in Chapter 14 of the FreeBSD Handbook:
[Jails](http://www.freebsd.org/doc/en_US.ISO8859-1/books/handbook/jails.html).
The [jail(8)](http://www.freebsd.org/cgi/man.cgi?query=jail&sektion=8)
manpage is also a great resource.


If you are working with bhyve or XEN, be sure to first try to read the official documentation:
[Chapter 21. Virtualization: FreeBSD as a Host with bhyve](https://www.freebsd.org/doc/handbook/virtualization-host-bhyve.html) and
[XEN project hypervisor](http://www.xenproject.org/).

Before getting started, be aware of the following terminology, and how
it will be used;


- **Node**: A physical server that hosts the virtual
environment.
- **Jail**: An isolated environment, complete with its own
set of software and services. A jail is able to run any software that is
available to the OS installed in the jail (cli or graphical).
- **Cloud:** A farm/cluster of interconnected nodes, or a
full-fledged peer network (each node can do other tasks through **CBSD**)
- **Base:** In the context of **CBSD**, a copy
of the files in the FreeBSD base.
- **CBSD:** An entity that has control over the specified
node(s) and certain subsystems of FreeBSD. CBSD provides a unified way to
interact with and perform actions on the specified nodes or jails via the
provided API. **CBSD** also provides the ability to implement
and use [ACL](https://www.freebsd.org/doc/handbook/fs-acl.html),
and change permissions on specified resources.
- **$workdir:** The working directory on a **CBSD**
node that is initialized via the _cbsd initenv_ command on the
initial run. This directory is **/usr/jails** unless otherwise
specified.
- **$jname:** The name of a jail in the **CBSD**
environment.

A quick word about jails. As stated, most any software available to the
OS the jail runs can be ran inside of a jail. Server-side components such
as DNS, Apache/nginx, or postfix, can run isolated from the host. Perhaps
lesser known is graphical environments/applications can also run inside a
jail isolated from the host. For example, run an XServer or VNCServer,
then connect to it. A single application can be run from inside a jail,
and then connected to using Xforwarding. _firefox -display=REMOTEADDR:PORT_

The largest directory used by CBSD is where all of the data **CBSD**
uses is stored. This is the directory **$workdir**, and is a
symlink to **_/usr/jails_** by default. This directory
can be changed when necessary. $workdir is also the **CBSD**
user's home directory. To quickly enter this dir from any other path, pass
'~cbsd' to the **cd** command.


```
			% cd ~cbsd

```

There are two main directories used to store jail data. The deciding factor
for which directory is used depends on whether or not a newly created jail
should be able to write to it's base or not. This option is specified by
passing the flag **baserw=0 or baserw=1** when creating a new
jail.


**baserw=0;**

To create a jail with a readonly base, pass the flag **baserw=0**.
Instead of writing to the base, the new jail will use the standard
base from _$workdir/basejail/$basename_. Jails with a read only
base are stored in the directory **_$workdir/jails/$jname_**.
Any baserw=0 jail will mount the $basename through nullfs. This allows for
the easy upgrade of all baserw=0 jails, as upgrading the $basename jail
upgrades all of the jails using it. Another advantage is the fact that if a
read only jail is compromised, the attacker will be unable to modify anything
in base as it is read only.


**baserw=1;**
When a new jail is created with the flag **baserw=1**, the jail
will have the ability to write to it's own base. Jails with this ability
store data in the directory **_$workdir/jails-data/$jname_**.


**CBSD** uses the standard directories specified by jail(8).
This allows jails to migrated to or from any other jail management system
that also follows the standards set by jail. The goal for the directories where
jails are stored is to be consistent, and adhere to the jail standards. This
allows for the greatest compatibility.


**_Note:_** When using the jail type md, the directory $workdir/jails-data/$jname
will contain the image of the jail.


**_Note:_** When using ZFS, CBSD has the ability to unmount a jail's
data directory while the the jail is inactive. If a jail's data directory
is found to be empty, don't panic. (At least when the jail is inactive).
Check the output of the command;


```
			% zfs list

```

To access the data use;


```
			% zfs mount $jname_file_system

```

The second-largest directory in the **CBSD** hierarchy is
_$workdir/var/db/_. This directory is where the configuration files
for all of the jails created are stored. All jail settings are stored in the
**jails** table in an SQLite3 database. The symbolic link
_${workdir}/var/db/local.sqlite_ should always point to the correct/
current database. The table schema is described in the file
_${workdir}/share/local-jails.schema_. SQLite3 can be used to query
information about all jails on a node.


For example, to see all jails on the node, and their IP address' execute;

```
			% sqlite3 /usr/jails/var/db/local.sqlite "select jname,ip4_addr from jails"

```

The _$workdir/jails-system/_ directory serves as additional
storage for **CBSD** jail data.

For example: There may be configurator's services, files with the description
of the jails, traffic statistics, resources statistics, and so on.

Internal information for **CBSD** is stored in the $workdir/db directory.

For example: The information on the list of added nodes, inventory of both
the local and remote nodes, and so on.


One important thing to note in regards to security are the directories
**_${workdir}/.rssh_ and _${workdir}/.ssh_**.
These dirs contain the private RSA keys for the remote user **CBSD**
nodes (.rssh) and the local nodes(.ssh). Make sure that the data in these
directories are not available to other users of the system. For more
information, please see the article about
[GELI encryption](cbsd_geli.md). By default, the key can
be read only by a system **CBSD** user.


Finally, be sure to read about the modifications that **CBSD**
does to the system. This [page](custom_freecbsd.md) describes
all of the modifications that are carried out by **CBSD**
scripts after installing on a FreeBSD system.


## Modules

The functionality of **CBSD** can be extended by activating additional modules that anyone can write.

Each module is a directory located in the _/usr/local/cbsd/modules_ path. To activate a module, you must add its name
into the _~workdir/modules.conf_ configuration file and re-initialize **CBSD** with: cbsd initenv

A good example of using additional modules is [ClonOS project](https://clonos.convectix.com),
which, using modules, adds the VNC terminal to jail, add notifications via web socket transport, add helpers for deploying services in containers, etc.

## Configurations and Settings

**CBSD** is a highly configurable and customizable framework, which makes it an extremely flexible and versatile solution when used in a wide range of tasks.

Take a look at the contents of the _~workdir/etc/defaults/_ directory.
It contains global settings (for example: color scheme, logging setup) and/or configuration files for single commands (for example: blogin, bstart, jclone, etc).

You can reassign these settings in the FreeBSD-way, by writing the changes in the file of the same name, but placing it a higher level in the directory _~workdir/etc/_ directory

Similarly, as you do with the FreeBSD configuration and _/etc/defaults/_ files

## Multiple operation by jname as mask

Most of the **CBSD** commands are support jname as mask.

For example, if you want to perform a similar operation on a group of jails (e.g: jail1, jail2, jail3), you can use **jname='jail\*'**

Another example:

```
cbsd jset jname='*' ver=native
cbsd jset jname='*' ver=native astart=0 devfs_ruleset=4   [..]
cbsd jexec jname='jail*' file -s /bin/sh
cbsd pkg jname='myja*l*' mode=install  ca_root_nss nss
cbsd jstop jname='*'
cbsd jstart jname='lala*'

```

![](http://www.bsdstore.ru/gif/jnamemask.gif)

## A brief summary of the filesystem hierarchy **CBSD**

**${workdir}/.rssh/**This directory stores the private keys of remote nodes. The files are added and removed via the command **cbsd node****${workdir}/.ssh**This directory stores the private and public keys of the nodes. The directory is created during initialization with the command **_cbsd initenv_**. This is also where the public key comes from when the command **cbsd node mode=add** is issued to copy the pub key to a remote host. The Key file name is the md5 sum of the nodename. In addition, this key pair is used by default when working with cloud images of virtual machines, For example, in the operations 'cbsd bexec, cbsd bscp, cbsd blogin'${workdir}/basejailThis directory is used to store the bases and kernels for FreeBSD that are used when creating baserw=0 jails. These are generated via cbsd buildworld/buildkernel, cbsd installworld/installkernel, or cbsd repo action=get sources=base/kernel)${workdir}/etcConfiguration files needed to run **CBSD**${workdir}/exportThe default directory that will be stored in a file exported by the jail (a cbsd jexport jname=$jname, this directory will file $jname.img)${workdir}/importThe default directory containing data to be imported to a jail (a cbsd jimport jname=$jname, will be deployed jail $jname)${workdir}/jailsThis directory contains the mount point for the root jails that use baserw=0.**${workdir}/jails-data**This directory stores all jail/VM data. Backup these directories to take a backup of the jails (including fstab and rc.conf files). Note: if a jail uses baserw=1, these directories are the root of the jail when it starts. Pay attention! when using ZVOL, in fact, the virtual disk are located in/as ZVOL and jails-data/ENV has only symbolic links - please take into consideration this when planning backup/DR!. This directory (or volume, which indicate symlinks to) is subject to backup copying if you want to restore your virtual environment in the case of failures.**${workdir}/jails-fstab**The fstab file for the jails. The syntax for regular FreeBSD with the only exception that the path to the mount point is written relative to the root _jail_ (record **/usr/ports /usr/ports nullfs rw 0 0** in the file fstab.$jname means that of the master node directory /usr/ports will be mounted at startup in ${workdir}/jails/$jname/usr/ports)${workdir}/jails-rcconfrc.conf files for jail creation. These parameters can be changed using $editor, or via the command **_cbsd jset $jname param=val_** (eg _cbsd jset jname=$jname ip="192.168.0.2/24"_). To change these settings, the jail should be turned **off**. **${workdir}/jails-system** This directory may contain some helper scripts related to the jail (eg wizards to configure, configurators, etc) as well as the preserved jail traffic when using ipfw and its description. This catalog participates in jimport/jexport operations and migration of jail. This catalog is subject to backup copying if you want to restore the **CBSD** during failures. ${workdir}/var This directory contains system information for **CBSD**. For example, in ${workdir}/var/db is an inventory of local and remote nodes that were added. **${workdir}/var/db** The main and the most important SQLite3 CBSD base, it is indicated by a sympathetic link **local.sqlite**. This catalog is subject to backup copying if you want to restore the **CBSD** during failures. /usr/local/cbsd A copy of the original files installed by the **CBSD** port. The working scripts for sudoexec can also be found here.

For example, for manual transfer of a container/VM from the **CBSD** structure, you need a SQLite3 database indicated by a symbolic link ~cbsd/var/db/local.sqlite. Its contents (table jails, bhyve, xen ..) is responsible for the list of registered environment, for example:

```
sqlite3 ~cbsd/var/db/local.sqlite "SELECT * from jails";
sqlite3 ~cbsd/var/db/local.sqlite ".schema jails";
sqlite3 ~cbsd/var/db/local.sqlite ".tables";
sqlite3 ~cbsd/var/db/local.sqlite ".dump";

```

Also, you need a rootfs container in the ~cbsd/jails-data/JAILNAME-data directory

As for virtual machines, you need to additionally check whether the discs of virtual machines ZVOL-based ( zfs list \| egrep "dsk \[0-9\]+.vhd"), as well as, be sure to copy ~cbsd/jails-system/ENVNAME/, where there are a virtual machine settings in the local.sqlite file:

```
sqlite3 ~cbsd/jails-system/VMNAME/local.sqlite "SELECT * from settings";
sqlite3 ~cbsd/jails-system/VMNAME/local.sqlite ".tables";
sqlite3 ~cbsd/jails-system/VMNAME/local.sqlite ".dump";

```

## Counting jail traffic

**CBSD** uses the **count** ruleset of
[**ipfw**](https://www.freebsd.org/doc/en/books/handbook/firewalls-ipfw.html)
filter to count jail traffic. **CBSD** sets the number of counters
in the **99 — 2000** range. The range can be easily adjusted in
cbsd.conf if this interfes with existing rules. Be mindful when changing
firewall rules. **CBSD** "takes ownership" of the rules in the
range given. In otherwords, if there are other rules already in place using
the specified range, there is the posibility that **CBSD** could
delete and re-add the rules in the range. This means all rules in the range
would be deleted, but only the **CBSD** rules would be added
back in.


Read more about [counting jail traffic](wf_jailtraffic_ssi.md).


## Expose: tcp/udp port forwarding from master host to jail

**CBSD** uses the **fwd** ruleset of
**ipfw** to configure port forwarding. **CBSD**
sets the number of counters in the **2001 - 2999** range. This
range can easily be changed in cbsd.conf if need be. Again, always be mindful
when changing firewall rules. Make sure no rules conflict with the range
configrured for **CBSD** to use.


Read more about [expose](wf_expose_ssi.md).


## About rsync-based copying jail data between nodes

**CBSD** offers a wrapper to rsync called cbsdrsyncd. If
**cbsdrsyncd** is activated, please keep in mind that there
is the standard **rsyncd(1)** daemon running that looks at
the specified _$jail-data_ directory, and is protected by the rsync
password. **CBSD** generates a strong password via the
following command;


```
		 head -c 30 /dev/random | uuencode -m - | tail -n 2 | head -n1

```

**CBSD** transmits data through the rsync daemon over
port 1873/tcp. Please secure this port from any traffic excpet for remote
**CBSD**, or use encrypted communication between the nodes
using something like IPSec.


## ANSII Color

**CBSD** displays output using colorized text by default
using ANSII escape sequences. Doing so helps important information standout.
If the colors are found to be unpleasant, or interfere with using output
from commands or utilities available in **CBSD**, colors can
be disabled by setting the environment variable NOCOLOR=1.

For example, issuing the command;


```
			% env NOCOLOR=1 cbsd jls

```

will disable the use of color in the output of the names of the jails.


## If something went wrong

While the **CBSD** project strives to be bug free, like
any software, bugs happen. If a component or tool that is part of
**CBSD** crashes, or returns unexpected data or behaviour,
[CBSD command debuging](cmdsyntax_cbsd.md#cmddebug)
can be enabled. If the bug is reproducible, and an actaul bug discovered,
please report the issue via e-mail: **CBSD** _at_ **bsdstore.ru**, or better yet submit a pull request that
identifies the issue found, and contains the code to resolve the issue.


## Taking backups of CBSD virtual environment.

**Taking a backup**

Any sys admin worth their salt would agree that taking regular backups is a
must to ensure data is safe. To properly backup the virtual environments on
the node, the following directories must be included (The description of each
of these directories is in the table above;


- ${workdir}/var/db
- ${workdir}/jails-fstab
- ${workdir}/jails-system
- ${workdir}/jails-data

