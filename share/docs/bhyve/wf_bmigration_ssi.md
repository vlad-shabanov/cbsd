# Bhyve Live migration

## command: bmigrate

The implementation of this feature in **bhyve** is grateful to the Politehnica University of Bucharest and in particular: **Elena Mihailescu** and **Mihai Carabaș**.

At the time this page was published, this functionality was not available in the FreeBSD codebase and was obtained from the project page on [GitHub (FreeBSD-UPB)](https://github.com/FreeBSD-UPB)

This functional is a continuation of the [checkpoints](http://www.bsdstore.ru/en/13.0.x/wf_bcheckpoint_ssi.html) functional

Primary requirements:

At the moment, the necessary condition is the presence of DFS, which in the case of **CBSD** should not be a problem ( more: [lack of binding to ZFS](http://www.bsdstore.ru/en/articles/cbsd_with_dfs.html)). At the moment, the work tested on **NFSv3,v4** and **GlusterFS** ( **Ceph** in the testing process )

For a successful bhyve live migration procedure, you also need to have servers that are closest to the technical specifications (architecture, CPU). Currently, the bhyve hypervisor does not support alignment of CPU instructions (editing and customization of CPUID) in the presence of different processors (different generation/models)

Besides **CBSD** nodes, which are exchanged virtual machines, must be added to the **CBSD** cluster via the [node](http://www.bsdstore.ru/en/13.0.x/wf_node_add_ssi.html) command.

In the process of live migration, the node-source uses the functionality to create a deferred task on the node-destionation through **cbsd task**, so make sure that you have a running process 'cbsdd' ( is controlled by the **cbsdd\_enable=YES** parameter in the _/etc/rc.conf_ config file)

To migrate, use the command **bmigrate**, which has two arguments - the name ( **jname**) a moving virtual machine and the destination node ( **node**)

![](http://www.bsdstore.ru/img/bmigration1.png)

During the migration, the script performs a preliminary check for the compatibility of the nodes, including the presence of common/shared directories (this is **jails-data, jails-rcconf, jails-system** directories in the **CBSD** working environment)

![](http://www.bsdstore.ru/img/bmigration2.png)

A small demo at an early stage of development: [Youtube::FreeBSD bhyve live migration, first overview](https://youtu.be/-IYNSBhtJqw)

A small demo at an early stage of development: [Youtube::FreeBSD bhyve live migration: FreeBSD guest + networking (no sound)](https://youtu.be/EyEtw8vEcxE)

A small demo at an early stage of development: [Youtube::FreeBSD bhyve live migration: Linux Debian guest + networking (no sound)](https://youtu.be/q94ZaP2Nqvo)

See also: [checkpoints](http://www.bsdstore.ru/en/13.0.x/wf_bcheckpoint_ssi.html), [taskd](http://www.bsdstore.ru/ru/cbsd_taskd.html), [CPU topology](http://www.bsdstore.ru/en/13.0.x/wf_bcpu_topology_ssi.html)


