# Copying files from/to jail filesystem

## jailscp command

```
	% jail1:remotefile1 localfile1 [ localfile2 jail2@:remotefile2 ]
```

The **cbsd jailscp** command allows you to copy files between your host filesystem that of a jail. The syntax is similar the one used by [scp(1)](http://man.freebsd.org/scp/1)

Copying files to and from remote jails is only possible if the necessary ssh keys are available. When a **rootkeyfile** for remote node is present, the connection will default to using **root** privileges rather than those of the **cbsd** user. This allow you to write files to any directory in the jail, not those owned by **cbsd**

**Example:**

```
cbsd jailscp

```

![](http://www.bsdstore.ru/img/jailscp1.png)


