# Virtual Machine EXport

## command: bexport

```
	% cbsd bexport
```

**Description**:

Export the VM file (\* .img). The argument **jname** the name of the virtual machine. img-file is saved in the directory _$workdir/export_. The original VM after exporting remains unchanged.

**EXample** (export VM debian1 to $workdir/export/debian1.img):

```
	% cbsd bexport jname=debian1
```
