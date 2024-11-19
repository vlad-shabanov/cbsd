# Virtual Machine Configuring

## commands: bconfig, bset

```
	% cbsd bconfig
	% cbsd bset
```

**Description**:

Configuring the Virtual Machine Settings

Each virtual machine **CBSD** stores the settings in the SQLite3 database. To change the settings of a VM can serve command **cbsd bconfig**, TUI launches menu for changing the basic settings.

Description of relevant parameters, you can read on the page [VM create](http://www.bsdstore.ru/en/13.0.x/wf_bcreate_ssi.html)

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

