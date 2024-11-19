# Virtual Machine Renaming

## command: brename

```
	% cbsd brename
```

**Description**:

Performs renaming the virtual machine and the corresponding directory data in the new name. Can be executed only on the inactive virtual machine. As required parameters:

- **old** — old VM name
- **new** — new VM name

**Example** (renaming debian1 VM into debian50:

```
cbsd brename old=debian1 new=debian50
```
