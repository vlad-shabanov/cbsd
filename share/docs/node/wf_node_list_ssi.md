# List of nodes

## node mode=list command

```
	% cbsd node mode=list
```

The command **cbsd node mode=list** show name of the nodes RSA which keys were received through **cbsd node add**. Too most it is possible to receive, having seen the catalog _$workdir/.rssh_, in which there are keys.

Through argument **display** you can specify the fields for output data. If **display** is not specified, the value takes from _$workdir/etc/defaults/node.conf_ file, which you can change at its discretion via _$workdir/etc/node.conf_

**Example:**

```
cbsd node mode=list
```

![](http://www.bsdstore.ru/img/nodelist1.png)
