# Virtual Machine Import

## command: bimport

```
	% cbsd bimport
```

**Description**:


Import from image (\* .img) virtual machine. The argument **jname** the full path to the file or the name of the image (without extension . **img**), which must be located in the appropriate directory ( _$workdir/import_). **img**-file after the import is still in place. Additionally, you can import a virtual machine from an image in the alternate name (for example, you want to deploy the image of the debian1 from debian1.img, however, your system already has a VM with the same name). In this case, use the argument **newjname**

**Example:**

Import virtual machine from the file _/tmp/debian1.img_:

```
cbsd bimport jname=/tmp/debian1.img
```

Import debian2.img placed in _$workdir/import_ under new name **debian5**:

```
cbsd bimport jname=debian2 newjname=debian5
```

