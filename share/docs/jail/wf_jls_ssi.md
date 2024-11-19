# Jails list

## jls command

```
	% cbsd jls
```

**Description**:


Show the lists of environment on a local node or for all nodes. You can adjust the displayed fields through the argument **display**.
If **display** is not specified, the value takes from _$workdir/etc/defaults/jls.conf_ file, which you can change at its discretion via _$workdir/etc/jls.conf_.
To hide the header, use the argument **header=0**.

All possible options for the sample described in the _$workdir/share/jail-arg_ file

In addition to the basic CBSD values, you can add your own fields and values for output with the jls command. More on this: [environment custom facts](http://www.bsdstore.ru/en/custom_facts.html)

**JID**Jail ID**JNAME**environment name**IP4\_ADDR**list of assigned IP addresses (IPv4,IPv6)**HOST\_HOSTNAME**environment FQDN**PATH**root filesystem for jail**STATUS**On (running), Off (stoped), Unregister (missing in the SQL database)

For **vnet**-based environments with a virtual stack, the **REAL\_IP4** field for displaying the actual IP addresses inside the environment may also be useful to you.

Note: Jail in the unregister status may be insert to SQL database via command: **cbsd jregister**.
If remote nodes are added to the local server, you can display all env in the farm through:

```
cbsd jls alljails=1
```

or:

```
cbsd jls alljails=1 shownode=1
```

for output with node name where jail are hosted. In the output from cbsd jls alljails, it is possible to see only active jails (in status On)

**Another example:**

```
cbsd jls display=jname,ip4_addr,vnet,real_ip4,astart
cbsd jls header=0 display=jname,mycustom1,mycustom2,status
```

![](http://www.bsdstore.ru/img/jls1.png)

![](http://www.bsdstore.ru/img/jls2.png)


