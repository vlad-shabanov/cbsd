# Building and upgrading bases

Commands:

```
	% cbsd srcup
	% cbsd buildworld
	% cbsd installworld
	% cbsd world
	% cbsd bases
	% cbsd removebase
```

## General information

_remark_: "base" refers to a directories containing a full copy/hierarchy of FreeBSD OS files.

The directory for the base directory is located in the ~cbsd/basejail/ directory and has the form base\_'arch '\_' target\_arch '\_' ver ', for example: _/usr/jails/basejail/base\_amd64\_amd64\_14.1_

You can view the bases registered in CBSD and their versions via command:

```
	% cbsd bases

```

For the creation of new jails (or starting jails which require a RO mounted base), a copy of base is necessary.

If you created a container and the base files are missing, when you start CBSD automatically offers several options for obtaining the base of your choice, this is:

- Download the base archive over the network. Depending on the fbsdrepo=0,1 setting in initenv-tui, ONLY official FreeBSD repo will be used, or
first the CBSD project repository will be used and if there is no archive on it - the official FreeBSD repository after.

- If you create a container of the same architecture and version as your hoster system, CBSD can get base without downloading on the network and without compilation,
by constructing a copy of the base from your files
- Get the base by unpacking the .txz archive. In this case, you specify the path where the archive is located
- Compile the database yourself, running the command sequence: cbsd srcup and cbsd world. This is a very long process.

Thus, you control where and how files will be received, depending on your level of trust and security requirements

![](http://www.bsdstore.ru/img/bases1.png)

Described below are the steps needed to get the initial FreeBSD sources from the project's SVN repository ( [http://svn.freebsd.org](http://svn.freebsd.org/)), its compilation and installation in a separate directory which will act as base. These actions are necessary, if you don't want to use pre compiled versions available on [ftp://ftp.freebsd.org](ftp://ftp.freebsd.org) or [http://bsdstore.ru](http://www.bsdstore.ru), which can be retrieved with the **cbsd repo** command. Also, by manually building becomes possible to not only get releases, but also individual branches.

## Sources and configuration of receiving base

Starting with the **CBSD** 12.0.4, You can adjust the default base acquisition method and customize sources. The configuration file is responsible for this behavior: _~cbsd/etc/defaults/FreeBSD-bases.conf_ (or ~cbsd/etc/default/HardenedBSD-bases.conf)

To make changes, copy this file to the directory _~cbsd/etc/_

The configuration file supports $var and $arch variables; thus, you can more flexibly customize sources, for example:

```
auto_baseupdate=0
default_obtain_base_method="extract repo"
default_obtain_base_extract_source="/nfs/bases/${platform}/base-${arch}-${ver}.txz"
default_obtain_base_repo_sources="https://my.repo.local/bases/${platform}/${ver}/base-${arch}.txz https://mirror-service.local/base-${platform}-${ver}-${arch}.txz"

```

Set CBSD to choose the default method for retrieving the database from the local file system (extract). At the same time, the way in which it will perform the search and provided that CBSD is running on the FreeBSD platform and you are creating a container with the version of the base 12.1 and the architecture amd64, should look like this: **/nfs/bases/FreeBSD/base-amd64-12.1.txz**

In the absence of this file, **CBSD** will attempt to get the archive from: https://my.repo.local/bases/FreeBSD/12.1/base-amd64.txz and https://mirror-service.local/base-FreeBSD-12.1-amd64.txz

This feature, together with the **inter=0** key (non-interactive execution of CBSD commands) allows you to automate the process of deploy of containers using its own infrastructure

The auto\_baseupdate params adjusts the automatic update of the received base to the latest patch level ( FreeBSD 12.0-p1, FreeBSD 12.0-p2 ..) via freebsd-update (or hbsd-update for HardenedBSD platform) tools.

Please use auto\_baseupdate=1, if you want to automatically accept patches when getting a base

As an example of use case, you can look at the integration of **CBSD** with the [Cirrus CI](https://cirrus-ci.org/) service in [Issue #367](https://github.com/cbsd/cbsd/issues/367)

## Base obtaining

building or updating consists of the following three steps:

- Receiving or updating of the initial FreeBSD source code through **cbsd srcup**:
Receiving or updating of an source code for that FreeBSD version at which in the moment works the current node:


```
cbsd srcup
```


Receiving or updating of an source code for the concrete FreeBSD version, for example, for 14.1:


```
cbsd srcup ver=14.1
```


Receiving or updating of source code for STABLE (for example, for RELENG\_11):


```
cbsd srcup ver=14
```


These commands, at existence of source code in SVN, will fill catalogs _$workdir/src/src\_$ver_
- Compilation of source code via **cbsd buildworld**:
The result received from a step 1, can be used for base compilation. World compilation, at existence _$workdir/src/src\_$ver_ dir for current node version:


```
cbsd buildworld
```


Thus, objective files, at existence from the last build, won't be cleaned (make clean). If in the repeated builds there was a unsuccess, it is meaningful to force to clean old system **.o**-files with clean params:


```
cbsd buildworld clean=1
```


By default, multiline assembly, where number of **job** s equally to number of cores (sysctl -n hw.ncpu). The num ofr parallel jobs is regulated through the **maxjobs** parameter, for example, for build in one job:


```
cbsd buildworld maxjobs=1
```


At compilation, the personal [src.conf(5)](http://www.freebsd.org/cgi/man.cgi?query=src.conf&sektion=5) can be used , in which it is possible to specify standard KNOBS-y. The src.conf file should placed in the catalog $workdir/etc/ and have name **src-** _basename_ **.conf**, where basename — any name of configuration. For example, in default **CBSD** installation there are file **/usr/jails/etc/src-lite.conf**. For base build with certain src.conf, it is necessary to specify _basename_ parameter:


```
cbsd buildworld basename=lite
```


The next operation over building result (installworld), also should contain _basename=_ parameter, as the result of building will be placed in _$workdir/basejail/base\_$basename\_$arch\_$ver_ directory, where $basename — is name of configuartion.
Other params of buildworld:

  - _ver_ — to specify the concrete version of source code
  - _arch_ — build for concrete architecture
  - _stable_ — build for RELENG\_X branch
- Base installation via **cbsd installworld**:
The result received from a step 2, it is possible to use for installation of the new base in _${workdir}/basejail/base\_\*_ directory, from where it will be already mounted in the RO mode, or to be formed a new jails.
For install of files for the current version of system:


```
cbsd installworld
```


Installation of files for version 14.1 builded with src.conf named as _lite_:


```
cbsd installworld basename=lite ver=14.1
```


other params for installworld:

  - _arch_ — install for concrete architecture
  - _stable_ — install for RELENG\_X branch

**cbsd world** command is sequence for **cbsd buildworld && cbsd installworld**

When you have the base in _${workdir}/basejail_,you can go to the [jail creation](http://www.bsdstore.ru/en/13.0.x/wf_jcreate_ssi.html).

## Deleting the base

The base can be removed via cbsd removebase command.
By default, the version and architecture of your hoster is inherited.
If the versions and/or architecture do not match, use the previously described arguments ver, arch, target\_arch and stable. For example:

```
cbsd removebase
cbsd removebase ver=11 stable=1
cbsd removebase ver=14.1 stable=0 arch=i386 target_arch=i386
```
