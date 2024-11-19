# Jail export

## jexport command

```
	% cbsd jexport
```

**Description**:

**Attention:** command execution allows on jail in status **On**. However it is necessary to remember (especially for jail with databases) when you import such jails, with a high probability it is possible to got problems with inconsistency filesystem in jails, old .pid files that can break work of the imported jails

Export jail into file (\*.img). In **jname** arguments you can set jail for export. img-file stored in _**$workdir/export**_ directory. Original jail after exports is not modified

You can control compress level via **compress** arguments

CBSD use [xz(1)](https://man.freebsd.org/xz/1), tools for compress images and you can learn in man page about compress diffrence between compress level.

By default CBSD use **compress=6**. You can disable compression with **compress=0**

**Example** (export mysqljail jail to $workdir/export/mysqljail.img):

```
jexport jname=mysqljail
```

## Exceptions for jexport

There are situations where you do not want to include one or another information from the container in the exported image.
For example, if you are exporting a working container having a mounted port of ports in /usr/ports.

In this case, you can help the **jexport\_exclude** parameter, which can be specified globally in the jexport.conf configuration file
(just copy default jexport.conf from _/usr/local/cbsd/etc/defaults/_ to _~cbsd/etc/_ and adjust the value for **jexport\_exclude**

If you want to specify alternative exclude for a specific Jail, copy this file to the _~cbsd/jails-system/ **$jname**/etc_ / directory.
In this case, when you make an **jexport** call, these exceptions will be applied for **$jname** container only.

Finally, you can do exclude without a configuration file at all, just listing all exceptions (space separated) as the **jexport\_exclude=** parameter when calling the **jexport** script, for example:

```
cbsd jexport jname=jail1 jexport_exclude="/var/run/* /usr/ports"
```

In this example, ports tree directory will not copy to image: since this data can always be mounted from the host.
And also, eliminates the presence of dead **.pid** files that can affect the correctness of the launch of the container's services after importing.

Exceptions are indicated relative to the root directory of the container.
So, if you want to migrate to another host **jail1** container configuration using jexport/jimport and do not want to copy any of its data, simply specify **/** as an exception:

```
cbsd jexport jname=jail1 jexport_exclude="/"
```

![](http://www.bsdstore.ru/img/jexport1.png)


