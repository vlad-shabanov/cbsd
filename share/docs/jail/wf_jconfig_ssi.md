# Jail config

## jconfig, jset commands

```
	% cbsd jconfig
	% cbsd jset
```

**Descriptions**:

Cconfiguration parameters jail

Each **CBSD** jail stores the settings in SQLite3 database. In addition, _$workdir/jails-fstab/_ may have fstab files (see below). To change the settings of jail can serve **cbsd jconfig** command, which is runs the TUI menu to change basic settings:

## Jails IP address

IP addresses that are bound to the jail sets in **ip4\_addr** parameter. As an IP may serve as IPv4, and the IPv6 address. When starting and stopping jail, working with IP may take place in two modes:

- automatic on-the-fly creation of IP addresses for the jail at the time of launch + automatic removal from the interface IP when stopping
- the use of previously initialized IP addresses.

When for jails assigned to more than one address, they should be listed separated by commas without spaces. IP can include network prefix specified through IP/ **prefix**. By default, aliases created with the prefix /32, that may not be appropriate if the jail uses a separate subnet from the network server — in this case, the correct **/prefix** is needed.

The parameter that controls this behavior pointed by **interface** parameter. When **interface=0**, jstart and jstop will not be called ifconfig alias and ifconfig -alias, respectively. If its value is **auto** or name of network nics, this command will be executed:

```
ifconfig interface ips alias
```

and when jail stop:

```
ifconfig interface ips -alias
```

Be careful with this option, if you have only one IP for server that is used and this IP is assigned to the same jail, when stopping jail, ip address of the server will be removed automatically that will make the server unavailable. In this case, you need to use:

```
interface=0
```

For example, to run the configuration tool for the **jail1**, run:


```
cbsd jconfig jname=jail1
```

## Mounting File Systems in jail

Each jail can have your fstab file, which lists the file systems that are mounted jail is startup. System records (they are managed by **CBSD** and edit this file inappropriately) are located in the file _$workdir/jails-fstab/fstab_ and
the same syntax to format the file system **/etc/fstab** with the exception that as a mount point is the path relative to the root jail, not the master system.

For user entries, you can use the file in the same directory with the extension _.local_.
For example, if you want to make between **jail1** and **jail2** a shared directory (via nullfs), which physically located in the master node (e.g.: _/home/sharefs_),
the files _$workdir/jails-fstab/jail1/fstab.local_ and _$workdir/jails-fstab/jail2/fstab.local_ should have:

```
/home/sharefs /home/sharefs nullfs rw 0 0
```

If you want to mount tmpfs to /tmp dir in jail1 (those actually in /usr/jails/jails/jail1/tmp), then the entry in the _$workdir/jails-fstab/jail1/fstab.local_ should have:

```
tmpfs /tmp tmpfs rw 0 0
```

both entries perform the same action as commands:

```
mount -t nullfs -o rw /home/sharefs /jailroot/home/sharefs
mount -t tmpfs -o rw tmpfs /jailroot/tmp
```

If you want to mount into **jail2** a directory from **jail1**, jail1 path should point to the directory containing the data jail1 (and their mount points ${workdir}/jails/jail1). For example entry _$workdir/jails-fstab/jail2/fstab.local_:

```
/usr/jails/jails-data/jail1-data/usr/local/www /usr/local/www nullfs ro 0 0
```

Make the shared directory /usr/local/www between jail1 and jail2, but it will jail2 in read-only mode

There are flaws when mounting via **mount -t /type/** does not work or mounting must be done in a non-standard way. In this case, use the 'external' reserved word, where the 'source' field will point to your external script, the output of which should be a mounted directory.

An example of such an entry in fstab.local:

```
/root/ntfs-3g.sh        /home/web/downloads external rw 0 0
```

and an example working script /root/ntfs-3g.sh for this entry: [ntfs-3g.sh](https://github.com/cbsd/cbsd/blob/v13.1.1/share/examples/jails-fstab/ntfs-3g.sh), which mounts an external USB drive directly into the container.

## Presentation of ZFS file systems in jail

If you want to attach separate ZFS filesystems in jail (ie, want to be able to perform in jail zfs mount), ZFS dataset must list in _$workdir/jails-fstab/$jname/fstab.local_ file with specifying the keyword zfs in FStype field. For example, if you want to present the file system ZFS: **zroot/jail1\_webfs** for jail jail1 _$workdir/jails-fstab/jail1/fstab.local_ must have:

```
zroot/jail1_webfs /home/web zfs rw 0 0
```

_Note:_ mount point (/home/web in this example) is not important

_Note:_ jail must have _allow\_zfs_ paramaters set to 1, what can be done via **cbsd jconfig jname=$jname**

In fact, it makes **CBSD** execute commands:

```
zfs set jailed=on $FS
zfs jail $jname $FS
```

when jail started and

```
zfs set jailed=off $FS
zfs unjail $jname $FS
```

when stoped.

## Change settings via the jset

Another possibility is to change certain parameters of the jail — use the command **cbsd jset**. Full list of possible arguments can be accessed through _--help_:

```
cbsd jget --help
```

For example, to change ip of jail1:

```
cbsd jset jname=jail1 ip4_addr="10.0.0.20/24,192.168.0.20/24"
```

_cbsd jconfig jname=jail2_

![](http://www.bsdstore.ru/img/jconfig1.png)

_cbsd jset_

![](http://www.bsdstore.ru/img/jconfig2.png)

## Custom scripts for starting and stopping action on jail

You can write your own scripts to be executed within the jail and in the master host on startup and shutdown of the environment. For this, the system directory of jail ( _**$workdir**/jails-system/ **$jname**/_ ) have the following directories:

- **master\_poststart.d** \- scripts for executing in the master host after jail starts ( _Be careful, because the scripts are is not jailed_)
- **master\_poststop.d** \- scripts for executiong in the master host after jail stops ( _Be careful, because the scripts are is not jailed_)
- **master\_prestart.d** \- scripts for executing in master host before jail starts ( _Be careful, because the scripts are is not jailed_)
- **master\_prestop.d** \- scripts for execution in master host after jail stops ( _Be careful, because the scripts are is not jailed_)
- **start.d** \- scripts for execution within jail when jail is starts. Analog of parameter **exec.start** from original _jail.conf_
- **stop.d** \- scripts for execution within jail when jail is starts. Analog of parameter **exec.stop** from original _jail.conf_
- **remove.d** \- (since CBSD 11.0.10) scripts for execution on remove command

Write scripts to the _master\_\\\*_ directories can be useful if at the start-stop jail you need to perform some action is not associated with content of environment - for example, create a ZFS snapshot, put rules in IPFW and etc.

In scripts, you can use CBSD variables, such as **$jname, $path, $data, $ip4\_addr**, for example, by placing a script (with execute permission) in _/usr/jails/jails-system/jail1/master\_poststart.d/notify.sh_:

```
#!/bin/sh
echo "Jail $jname started with $ip4_addr IP and placed on $path path" | mail -s "$jname started" root@example.net
```

You will receive notification upon startup cells by email: root@example.net

The functionality of execution custom scripts and the availability of variables in environments can play a big role in the integration of **CBSD** and external applications, such as **Consul**

As an example of use, see the article [Example of using CBSD/bhyve and ISC-DHCPD (Serve IP address in bhyve through pre/post hooks)](http://www.bsdstore.ru/en/articles/cbsd_vm_hook_dhcpd.html)

As an example of use, see the article [Example of using CBSD/jail and Consul (Register/unregister jail's via pre/post hooks, in Consul)](http://www.bsdstore.ru/en/articles/cbsd_jail_hook_consul.html)


