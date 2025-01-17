# Cloning a Virtual Machine

## command: bclone

```
% cbsd bclone
% cbsd brclone
```

**Description**:

Performs cloning a virtual machine to a new one. As required arguments, specify source/original VM through **old** and a new name as an argument **new**

**Attention:** Since 11.0.10 version, CBSD on ZFS-based hosters will be use ZFS clone features!

ZFS clone features is ultra fast operation (thanks to Copy-on-write), but imposes some restrictions - you will be dependent on the parent snapshot. If you try to remove parent environment, CBSD automatically executes the **zfs promote** command,
but when you works with snapshot independently - just keep it in your mind

You can control this behaviour via **clone\_method=** argument or, to set it globally, use _rclone.conf_ and _bclone.conf_ to overwrite settings from 'auto' to 'rsync':


```
% echo 'clone_method="rsync"' > ~cbsd/etc/rclone.conf
% echo 'clone_method="rsync"' > ~cbsd/etc/bclone.conf
```

When ~cbsd/etc/bclone.conf (for bclone) and ~cbsd/etc/rclone.conf contain:
```
clone_method="rsync"
```
Clone will not use zfs clone even on ZFS filesystem and you will get full copy via rsync
Example: Cloning a virtual machine in debian1 to debian2:
```
% cbsd bclone old=debian1 new=debian2
```
