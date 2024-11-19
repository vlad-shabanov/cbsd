# jail upgrade: etcupdate

## command: etcupdate

```
% cbsd etcupdate
```

**Description**:

To solve the _distribution_ file synchronization problem (this is basically the contents of the _/etc_ directory) when changing versions of the base system,
FreeBSD offers two utilities: [mergemaster(8)](http://man.freebsd.org/mergemaster/8)
and [etcupdate(8)](http://man.freebsd.org/etcupdate/8).
**CBSD** has a script for working with etcupdate under the same name.
By default **CBSD** creates a directory hierarchy for working etcupdate in the system directory of the database and each jail.
You can turn this off by overriding the etcupdate\_init parameter value to '0' (disabled) in the profile of your container or
globally via jail-freebsd-default.conf the configuration file in the _~cbsd/etc/_ directory.

What **CBSD** etcupdate does NOT:

- **CBSD** does not automatically launch file updates, as these operations may lead
to disruption of the services or unexpected changes in the files for you.
Some attempts at automatic merge operations may cause a conflict. In these cases, operator intervention is required.


What **CBSD** etcupdate does:

- **CBSD** automatically creates build/extract files for etcupdate to work correctly for each version
of the FreeBSD base. **CBSD** keeps track of the versions of your containers and the conformity of
distribution files, checking and displaying a corresponding message when the container starts up about the need for synchronization.


**CBSD** etcupdate wrapper allows you to do the following operations by repeating the keys of the original etcupdate(8)utility:

**\*** perform the etcupdate operation for container XXXX, from the old version of container XX to the new YY.
Since **CBSD** owns information about which version was and which version is now, the from = and to = parameters can be omitted:

```
  cbsd etcupdate mode=update jname=XXXX [from=XX to=YY]

```

**\*** run extract initialization for the FreeBSD database version XX.YY (see man etcupdate):
( **CBSD** from version 11.1.9 performs this operation automatically when receiving bases)

```
  cbsd etcupdate mode=extract ver=XX.YY [force=1]

```

**\*** run build initialization for version XX.YY (see man etcupdate):
( **CBSD** from version 11.1.9 performs this operation automatically when receiving bases)

```
  cbsd etcupdate mode=build ver=YY.YY [force=1]

```

**\*** start manual correction of conflicts when they are available for container XXX.
Use the EDITOR value variable to use your favorite editor (see man etcupdate):

```
  cbsd etcupdate mode=resolve jname=XXX

```

**\*** clean jail working directory from etcupdate files
(precedes mode = extract and mode = build operations):

```
  cbsd etcupdate mode=purge

```

In addition to this, **CBSD** backs up distribution files before performing the operation
update, and also regulates the rotation of archives in the container's system directory (~cbsd/jails-system/XXX/etcupdate).
You can control this behavior with the parameter etcupdate\_create\_backup= in the etcupdate.conf configuration file.

## FreeBSD-way config files best practices

In order to minimize the possibility of conflicts and thereby facilitate the synchronization of configuration files,
try to avoid editing basic system files such as /etc/inetd.conf, /etc/syslog.conf, /etc/ssh/sshd\_config,
/etc/newsyslog.conf and so on. etc. Instead, use configuration systems such as Chef, Salt, Puppet that manage the necessary configurations.
Alternatively, use the \_flags arguments in the standard /etc/rc.conf file to override your configuration files.
For example, instead of editing /etc/sshd/sshd\_config, you can override sshd options in /etc/rc.conf as follows:

```
 sshd_flags="-oUseDNS=no -oPort=22222 -oPermitRootLogin=no -oUseBlacklist=yes"

```

If the flags do not allow you to make the necessary settings in full,
copy the service configuration file to another location and redefine the path to it via \_flags.
For example, create the directory _/root/etc_, where your configuration files for inetd, blacklistd, syslogd, newsyslog will be located:

```
 mkdir /root/etc
 cp -a /etc/inetd.conf /root/etc
 cp -a /etc/syslog.conf /root/etc
 cp -a /etc/blacklistd.conf /root/etc
 cp -a /etc/newsyslog.conf /root/etc

```

Using keys/args, we redefine the configuration path for the above utilities:

```
 sysrc inetd_flags="-wW -C 60 /root/etc/inetd.conf"
 sysrc syslogd_flags="-ss -f /root/etc/syslog.conf"
 sysrc blacklistd_flags="-c /root/etc/blacklistd.conf"
 sysrc newsyslog_flags="-CN -f /root/etc/newsyslog.conf"

```

etc so on. A number of utilities also support the export of custom configurations to the
corresponding app.d directories. Use these tricks and your upgrade operations will be very easy.

## etcupdate examples

Let's create jail with the 11.3 base version:

```
cbsd jcreate jname=jail4 ip4_addr=DHCP ver=11.3

```

Change the version of the container base, thereby switching to an older version, for example to 12.1:

```
 cbsd jset jname=jail4 ver=12.1

```

Using the jls command, you can see the current version of the container base and the version of distribtion (etcver):

```
 cbsd jls display=jname,ver,etcver

```

When you start the container, you can see a message from **CBSD** about the discrepancy
between the files and the current database and an offer to use the command to update. Run it:

```
 cbsd etcupdate jname=jail4 mode=update

```

The command will update the files that are synchronized in automatic mode and will display a message about the
need to manually resolve conflicts if automatic mode has encountered difficulties.
By running resolve, we can fix the conflicts manually, bringing the file to the desired form:

```
  cbsd etcupdate jname=jail4 mode=resolve

```

![](http://www.bsdstore.ru/img/etcupdate1.png)

![](http://www.bsdstore.ru/img/etcupdate2.png)

![](http://www.bsdstore.ru/img/etcupdate3.png)

![](http://www.bsdstore.ru/img/etcupdate4.png)

![](http://www.bsdstore.ru/img/etcupdate5.png)

![](http://www.bsdstore.ru/img/etcupdate6.png)

Keep your systems up to date!


