# Jail cloning

## jclone command

```
	% cbsd jclone
	% cbsd jrclone
```

**Description**:

Creates a clone of a given jail. The command requires the name of the original jail (passed with **old**) as well as a name for the clone, specified with **new** and a FQDN (hostname) with **host\_hostname**. Optionally a new IP Address can be given with the **ip4\_addr** parameter (multiple IPs need to be separated by commas).

**Attention:** when you are using a ZFS-based system, **CBSD** will use ZFS cloning by default!

ZFS cloning features is ultra fast operation (thanks to Copy-on-write), but imposes some restrictions - you will be dependent on the parent snapshot.

You can control this behaviour via **clone\_method=** argument or, to set it globally, use _rclone.conf_ and _bclone.conf_ to overwrite settings from 'auto' to 'rsync':


```
echo 'clone_method="rsync"' > ~cbsd/etc/rclone.conf
echo 'clone_method="rsync"' > ~cbsd/etc/bclone.conf
```

## Custom data cloning jail method

You can create your own data cloning method using script(s) in the directory _$workdir/jail-system/$jname/clone-local.d_.

For example, creating a script to the $workdir/jail-system/$jname/clone-local.d/ directory with the name **myclone.sh** and the contents of the form:


```
#!/bin/sh
echo "My custom clone for: $jname -> $newjname
echo "${data} -> ${newdata}"
/bin/cp -Ra ${data} ${newdata}
```

You will force **CBSD** to execute your script as a data cloning method.

**Attention:** The script must always end with 0 exit code when success

Please note that standard variables are available in the script [CBSD variables](http://www.bsdstore.ru/en/13.0.x/wf_cbsd_variables_ssi.html). Additionally, these variables are exported: **$newjname**, **$newdata**

Why it is needed: if you use NAS or NFS as a storage and have access there, you can clone the data using NAS or directly on the NFS server, which will significantly increase the cloning speed and reduce unnecessary traffic. See: [Issue #373](https://github.com/cbsd/cbsd/issues/373)

**Example:**

cloning jail jail2 to jail3 with changes ip address **ip4\_addr** and name of hosts **host\_hostname**:

```
cbsd jclone old=jail2 new=jail3 host_hostname=jail3.my.domain ip4_addr=10.0.0.22/24
```

![](http://www.bsdstore.ru/img/jclone1.png)


