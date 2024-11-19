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
- Lang »  - [Русский](http://www.bsdstore.ru/ru/13.0.x/modules/pkg.d_ssi.html)
  - [English](http://www.bsdstore.ru/en/13.0.x/modules/pkg.d_ssi.html)
  - [Deutsch](http://www.bsdstore.ru/de/13.0.x/modules/pkg.d_ssi.html)

2020-10 upd: we reached the first fundraising goal and rented a server in Hetzner for development! Thank you for [donating](https://www.patreon.com/clonos) !

Attention! Current pages describe **CBSD** version **13.0.x**. If you are using an older version, please update first.

Please note: these commands support the mask (wildcard) as a jname, for example: jname='\*', jname='ja\*l\*'

Attention! I apologize for the automatic translation of this text. You can improve it by sending me a more correct version of the text or fix html pages via [GITHUB repository](https://github.com/cbsd/cbsd-wwwdoc).

# Wirking with packages and pkg(7) in jail via CBSD

## Command: pkg

```
			% cbsd pkg

```

**Description**:

**cbsd pkg** \- is wrapper around standart FreeBSD [pkg(7)](http://man.freebsd.org/pkg/7) tools to use **jname** argument for more comfort work with the jail from the master host

Via **mode=** argument indicating a needet action. Values can be:

- **add, install** \- to install packages
- **remove** \- to remove packages
- **bootstrap** \- init pkg (normally done in the jail one times on creating)
- **info, query** \- execute queries _info_ or _query_ with the same syntax pkg
- **update** \- execute pkg update
- **upgrade** \- execute upgrade
- **clean** \- execute clean to purge pkg cache

For some commands (clean, update, upgrade) it is permissible jname= to specify as mask for performing the operation simultaneously in several jails

Keep in mind that must first be specified parameters **mode** and **jname**. All that comes after - not analyzed and treated [pkg(7)](http://man.freebsd.org/pkg/7) as is.

In addition, please note that all operations are performed with the set environment variables **ASSUME\_ALWAYS\_YES=yes** and **IGNORE\_OSVERSION=yes** to suppress the interactivity that basically, you need to work in automated scripts. If for some reason this does not work for you, use [cbsd rexe](http://www.bsdstore.ru/en/13.0.x/wf_jexec_ssi.html) to work with pkg directly.

**Example1:** Update pkg index files inside ALL containers:

```
			% cbsd pkg mode=update jname='*'

```

**Example2:** Update ALL packages inside containers, whose name starts with redis\*:

```
			% cbsd pkg mode=upgrade jname='redis*'

```

**Example3:** Clear pkg cache in ALL containers:

```
			% cbsd pkg mode=clean jname='*'

```

**Example4:** Get installed packages for box1 and for all jails with jname mask 'jail\*' (in **CBSD 11.2.1+**):

```
			% cbsd pkg mode=query jname=box1 %o
			% cbsd pkg mode=query jname='jail*' %o

```

**Example5:** Install **bash, mc, wget** in mytest1 jail and **nginx-devel,mysql57-server,postgresql96-server,mc** for all jails with jname mask 'jail\*' (in **CBSD 11.2.1+**:

```
			% cbsd pkg mode=install jname=mytest1 bash mc wget
			% cbsd pkg jname='jail*' mode=install nginx-devel mysql57-server postgresql96-server mc

```

or that much better (in order to avoid the same name in different categories) indicate origin package, not the name:

```
			% cbsd pkg mode=install jname=mytest1 shells/bash ftp/wget misc/mc

```

**Example6:** Upgrade mc package in jail1:

```
			% cbsd pkg mode=upgrade jname=jail1 mc

```

**Example7:** Remove wget and lsof packages in box1 and mc from all jails with jname mask 'jail\*' (in **CBSD 11.2.1+**:

```
			% cbsd pkg mode=remove jname=box1 wget lsof
			% cbsd pkg jname='jail*' mode=remove mc

```

Copyright © 2013—2024 CBSD Team.

