# Attaching to terminal

## command: blogin

```
	% cbsd blogin
```

**Description**:

Execute **tmux** session, connecting to the console output of the virtual machine

To disconnect from the terminal (and off tmux session), press the key combination: **Ctrl+b, d** (Hold Ctrl press b, release both key and hit d)

**Example**:

```
cbsd blogin debian1
```

You can customize the command, redefining the action on you more suitable

This is achieved through the configuration file blogin.conf and the parameter **login\_cmd**.

The file can be placed for the individual environment in the directory _$workdir/jails-system/$jname/etc_ (for example, virtual machines belong to different system administrators, and you can use different logins when connecting via ssh),
and globally, overwriting the value from _$workdir/etc/defaults/blogin.conf_.
To do this, create a file with your configuration in the directory _$workdir/etc/_

With a custom call, you can use [CBSD variables](http://www.bsdstore.ru/en/13.0.x/wf_cbsd_variables_ssi.html) \- for this or that environment

For example, if you want instead of the standard behavior, when the blogin launched the VNC client, the file _$workdir/etc/blogin.conf_ can look like this:


```
login_cmd="su -m user -c \"vncviewer ${bhyve_vnc_tcp_ipconnect}:${vm_vnc_port}\""
```

If you want the ssh connection to occur, this file might look like this:


```
login_cmd="/usr/bin/ssh your_user@${ipv4_first}"

```

![](http://www.bsdstore.ru/img/blogin1.png)

![](http://www.bsdstore.ru/img/blogin2.png)

