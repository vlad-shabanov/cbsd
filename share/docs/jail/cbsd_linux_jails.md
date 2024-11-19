# CBSD and Linux jails

**CBSD** supports Linux Jails (through Linuxulator) and has a script for creating containers based on CentOS, Ubuntu and Debian distributions.

Of course, due to Linuxulator's restrictions, you will have to test the work of services in such containers very well,
so the **CBSD** authors support this possibility is provided, but the production installations are better to deploy in a Bhyve virtual machine, with which
CBSD also helps you.


An example of creating Linux containers via CLI:

```
cbsd jcreate jname=deb jprofile=debian_buster allow_raw_sockets=1
cbsd jcreate jname=ubu jprofile=ubuntu_focal allow_raw_sockets=1
cbsd jcreate jname=cent jprofile=centos_7 allow_raw_sockets=1
```

When the container first start, the script will work out, which will receive all the necessary distribution files from the Internet. Cache will be saved in the
**~cbsd/share/jail-\*-rootfs** directory, depending on the distribution.
