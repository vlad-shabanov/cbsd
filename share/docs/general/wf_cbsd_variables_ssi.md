[View source on GitHub](https://github.com/cbsd/cbsd)

FreeBSD virtual environment management and repository

- [About](http://www.bsdstore.ru/en/about.html)
- [News](http://www.bsdstore.ru/en/news.html)
- [Screenshots](http://www.bsdstore.ru/en/screenshots.html)
- [Tutorial](http://www.bsdstore.ru/en/tutorial.html)
- [Documentation »](http://www.bsdstore.ru/en/docs.html)  - [Articles by author's](http://www.bsdstore.ru/en/articles.html)
  - [Articles and press](http://www.bsdstore.ru/en/press.html)
- [Marketplace(Templates)](https://marketplace.bsdstore.ru)
- [Support the project](http://www.bsdstore.ru/en/donate.html)
- [bhyve.cloud](http://www.bsdstore.ru/en/bhyve-cloud.html)
- Lang »  - [Русский](http://www.bsdstore.ru/ru/13.0.x/wf_cbsd_variables_ssi.html)
  - [English](http://www.bsdstore.ru/en/13.0.x/wf_cbsd_variables_ssi.html)
  - [Deutsch](http://www.bsdstore.ru/de/13.0.x/wf_cbsd_variables_ssi.html)

2020-10 upd: we reached the first fundraising goal and rented a server in Hetzner for development! Thank you for [donating](https://www.patreon.com/clonos) !

Attention! Current pages describe **CBSD** version **13.0.x**. If you are using an older version, please update first.

Attention! I apologize for the automatic translation of this text. You can improve it by sending me a more correct version of the text or fix html pages via [GITHUB repository](https://github.com/cbsd/cbsd-wwwdoc).

# CBSD variables

**Description**:

**CBSD** is written taking into account the extensibility and flexibility, giving system engineers the opportunity to increase the functionality of the framework through modules or integration with various other
tools through hooks available at various stages of script execution, such as
such as [**pre/post start/stop hooks**](http://www.bsdstore.ru/en/13.0.x/wf_jconfig_ssi.html#execscript) or customization of blogin/jlogin command.

Here is a list of reserved variables participating in these stages and available for use:

${jname}(jail, bhyve, xen) :: the variable contains the name of the environment${allow\_devfs}(jail) :: parameter of jail(8)${allow\_dying}(jail) :: parameter of jail(8)${allow\_kmem}(jail) :: parameter of kmem CBSD${allow\_mount}(jail) :: parameter of jail(8)${allow\_nullfs}(jail) :: parameter of jail(8)${allow\_procfs}(jail) :: parameter of jail(8)${allow\_reserved\_ports}(jail) :: parameter of jail(8)${allow\_sysvipc}(jail) :: parameter of jail(8)${allow\_tmpfs}(jail) :: parameter of jail(8)${allow\_zfs}(jail) :: parameter of jail(8)${applytpl}(jail) :: parameter of applytpl CBSD${arch}(jail) :: arch of containers${astart}(jail, bhyve, xen) :: a sign of auto-start of the environment${b\_order}(jail, bhyve, xen) :: order/priority (weight) of environment launch${basename}(jail) :: name of the base for containers${baserw}(jail) :: parameter of baserw CBSD${bhyve\_flags}(bhyve) :: additional flags for bhyve${bhyve\_force\_msi\_irq}(bhyve) :: parameter of bhyve(8)${bhyve\_generate\_acpi}(bhyve) :: parameter of bhyve(8)${bhyve\_ignore\_msr\_acc}(bhyve) :: parameter of bhyve(8)${bhyve\_mptable\_gen}(bhyve) :: parameter of bhyve(8)${bhyve\_rts\_keeps\_utc}(bhyve) :: parameter of bhyve(8)${bhyve\_vnc\_resolution}(bhyve) :: VNC server resolution in 1024x768 format${bhyve\_vnc\_tcp\_bind}(bhyve) :: parameter of bhyve(8)${bhyve\_vnc\_tcp\_ipconnect}(bhyve) :: IP address for connecting a VNC client${bhyve\_wire\_memory}(bhyve) :: parameter of bhyve(8)${bhyve\_x2apic\_mode}(bhyve) :: parameter of bhyve(8)${cd\_vnc\_wait}(bhyve) :: parameter of bhyve(8)${childrenmax}(jail) :: parameter of jail(8)${cpuset}(jail, bhyve, xen) :: to which processor cores the environment is attached${data}(jail) :: actual container data path${devfs\_ruleset}(jail) :: parameter of jail(8)${emulator}(jail, bhyve, xen) :: name of the engine for virtualization/containirisation${emulator\_flags}(jail) :: flags for qemu${enforce\_statfs}(jail) :: parameter of jail(8)${exec\_consolelog}(jail) :: parameter of jail(8)${exec\_fib}(jail, bhyve, xen) :: applied routing table${exec\_master\_poststart}(jail, bhyve, xen) :: scripts for master\_poststart${exec\_master\_poststop}(jail, bhyve, xen) :: scripts for master\_poststop${exec\_master\_prestart}(jail, bhyve, xen) :: scripts for master\_prestart${exec\_master\_prestop}(jail, bhyve, xen) :: scripts for master\_prestop${exec\_poststart}(jail) :: scripts for poststart${exec\_poststop}(jail) :: scripts for poststop${exec\_prestart}(jail) :: scripts for prestart${exec\_prestop}(jail) :: scripts for prestop${exec\_start}(jail) :: scripts for start${exec\_stop}(jail) :: scripts for stop${exec\_timeout}(jail) :: parameter of jail(8)${floatresolv}(jail) :: parameter of baserw CBSD${hidden}(jail, bhyve, xen) :: visibility in the WEB interface ${host\_hostname}(jail) :: parameter of jail(8)${interface}(jail, bhyve, xen) :: to which network interface is the environment bound${ip4\_addr}(jail, bhyve, xen) :: the list of IP addresses of the environment specified in CBSD (comma separated)${maintenance}(jail, bhyve, xen) :: service mark${mdsize}(jail) :: the volume of the jail image, if jail is in the md-backend image${mkhostsfile}(jail) :: control of contents /etc/hosts${mount\_devfs}(jail) :: parameter of jail(8)${mount\_fdescfs}(jail) :: parameter of jail(8)${mount\_fstab}(jail) :: fstab file for the jail${mount\_kernel}(jail) :: mount kernel dir in jail?${mount\_obj}(jail) :: mount obj dir in jail?${mount\_ports}(jail) :: mount /usr/ports dir in jail?${mount\_src}(jail) :: mount /usr/src dir in jail?${nic\_hwaddr}(jail, bhyve, xen) :: MAC address of the virtual interface${path}(jail) :: root jail in the host file system${persist}(jail) :: parameter of jail(8)${protected}(jail, bhyve, xen) :: a sign of protecting the environment from uninstallation via the remove command${stop\_timeout}(jail) :: timeout of soft container stop${ver}(jail) :: FreeBSD base version for jail${virtio\_type}(bhyve) :: type of disk controller${vm\_cpus}(bhyve, xen) :: number of guest virtual cores${vm\_hostbridge}(bhyve, xen) :: ${vm\_iso\_path}(bhyve, xen) :: ${vm\_os\_profile}(bhyve, xen) :: ${vm\_ram}(bhyve, xen) :: amount of guest RAM${vm\_rd\_port}(jail, bhyve, xen) :: ${vm\_vnc\_port}(bhyve, xen) :: VNC port${vnc\_password}(jail, bhyve, xen) :: VNC password${vnet}(jail) :: vnet enabled jail?${ipv4\_first\_public}(jail, bhyve) :: first public IPv4 address of the environment${ipv4\_first\_private}(jail, bhyve) :: first private IPv4 address of the environment${ipv4\_first}(jail, bhyve) :: first (any) IPv4 address of the environment${ipv6\_first\_public}(jail, bhyve) :: first public IPv6 address of the environment${ipv6\_first\_private}(jail, bhyve) :: first public IPv6 address of the environment${ipv6\_first}(jail, bhyve) :: first (any) IPv6 address of the environment

Copyright © 2013—2024 CBSD Team.

