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
- Lang »  - [Русский](http://www.bsdstore.ru/ru/13.0.x/ezjail2cbsd_ssi.html)
  - [English](http://www.bsdstore.ru/en/13.0.x/ezjail2cbsd_ssi.html)
  - [Deutsch](http://www.bsdstore.ru/de/13.0.x/ezjail2cbsd_ssi.html)

2020-10 upd: we reached the first fundraising goal and rented a server in Hetzner for development! Thank you for [donating](https://www.patreon.com/clonos) !

Attention! Current pages describe **CBSD** version **13.0.x**. If you are using an older version, please update first.

Attention! I apologize for the automatic translation of this text. You can improve it by sending me a more correct version of the text or fix html pages via [GITHUB repository](https://github.com/cbsd/cbsd-wwwdoc).

# Migration jails from ezjail

Author: Nikita Druba aka **[LordNicky](https://github.com/cbsd/cbsd/issues/412)**

Original: via [Issue#412](https://github.com/cbsd/cbsd/issues/412)

Notes: this is ZFS-only solution

* * *

Migration from **ezjail** with basejail to **CBSD** with basejail can be doing at least by this easy way:

- Install **CBSD** and do an initial setup, like in the manual
- Create an empty jail with any name, for example: "jail1", make sure, that you uncheck "astart" option.; (https://www.bsdstore.ru/en/12.0.x/wf\_jcreate\_ssi.html;
- Stop the jail from **ezjail**, that you want to migrate and do a snapshot of it filesystem:


```
ezjail stop MyJail
```



```
zfs snapshot mypool/path/to/ezjail/MyJail@migrating
```

- Create jail in **CBSD** by jconstruct-tui and put to "zfs\_snapsrc" field path to snapshot of you jail:


```
mypool/path/to/ezjail/MyJail@migrating
```

- Check all other fields to set same ip, name and other settings. For network you can use syntax of ezjail, but instead "\|" you need to use "#".
- Proceed you new jail. If creation of jail was successfully, then go to the next step
- Run **migrating\_ezjail\_cbsd.sh** script from [ezjail2cbsd/](https://github.com/cbsd/cbsd_useful_stuff/tree/master/ezjail2cbsd). You can specify paths to directories of "migrating" and "example" jails in command:


```
migrating_ezjail_cbsd.sh path/to/cbsd/jail1 path/to/cbsd/MyJail
```


or use dialog interface and enter paths there.
- Run you migrated jail by cbsd command and enjoy.
- Also you can be needed to destroy **ezjail** filesystems, but before destroying them you need to promote clones in **CBSD** directory:

```
zfs promote mypool/path/to/cbsd/MyJail
```

```
zfs destroy -r mypool/path/to/ezjail/MyJail
```

(Be carefully with \[-R\] parameter, you need \[-r\].)

Copyright © 2013—2024 CBSD Team.

