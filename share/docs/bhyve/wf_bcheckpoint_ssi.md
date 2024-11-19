# Checkpoints, hibernation and pauses of the bhyve virtual machine

## Commands bcheckpoint, bsuspend, bpause

To use this feature the kernel (vmm.ko module) and bhyve userland utilites must be compiled with BHYVE\_SNAPSHOT flags. Please add into /etc/src.conf:

```
WITH_BHYVE_SNAPSHOT=yes

```

And add into kernel config file:

```
options         BHYVE_SNAPSHOT

```

Then rebuild the world/kernel.

The functionality of checkpoints and suspend of a virtual machine is the freezing of the virtual environment, saving the entire state to disk, from which you can return the system to its previous state without having to reboot the environment

## Checkpoints

In **CBSD** to work with checkpoints use the **bcheckpoint** command, which has the following syntax:

```
cbsd bcheckpoint [jname=] [mode=] [name=] [poweroff=]
```

where:

- \- **jname**: (required) the name of the environment, for example: freebsd1
- \- **mode**: (required) action relative to the environment: create (create checkpoint), list (list of checkpoints created), destroyall (destroy all checkpoints)
- \- **name**: (optional) specify an alternate checkpoint name, by default: checkpoint
- \- **poweroff**: (optional). When poweroff=1, **CBSD** will automatically shut down the virtual machine instantly
(via [bstop](http://www.bsdstore.ru/en/13.0.x/wf_bstop_bstart_ssi.html) **noacpi=1**) on the fact of checkpoint creation.

```
% cbsd bls
JNAME     JID    VM_RAM  VM_CURMEM  VM_CPUS  PCPU  VM_OS_TYPE  IP4_ADDR  STATUS  VNC_PORT
freebsd1  21923  1024    24         1        0     freebsd     DHCP      On      127.0.0.1:5900

% cbsd bcheckpoint mode=create jname=freebsd1
Waiting and sure that the info is written on the disk: 1/5
Waiting and sure that the info is written on the disk: 2/5
Waiting and sure that the info is written on the disk: 3/5
Waiting and sure that the info is written on the disk: 4/5
Waiting and sure that the info is written on the disk: 5/5
Checkpoint was created!: /usr/jails/jails-system/freebsd1/checkpoints/checkpoint.ckp

% cbsd bcheckpoint mode=create jname=freebsd1 name=after_update
Waiting and sure that the info is written on the disk: 1/5
Waiting and sure that the info is written on the disk: 2/5
Waiting and sure that the info is written on the disk: 3/5
Waiting and sure that the info is written on the disk: 4/5
Waiting and sure that the info is written on the disk: 5/5
Checkpoint was created!: /usr/jails/jails-system/freebsd1/checkpoints/after_update.ckp

% cbsd bcheckpoint mode=list jname=freebsd1
Created checkpoint for freebsd1:
after_update
checkpoint
```

Having created checkpoints, you can return to the desired state via the command **bstart** with **checkpoint** args

A short demo at an early stage of development on [Youtube](https://youtu.be/cyFDnmTKY_c)

## Suspend mode

WIP

## VM Pauses

WIP

