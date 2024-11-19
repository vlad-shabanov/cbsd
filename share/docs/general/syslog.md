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
- Lang »  - [Русский](http://www.bsdstore.ru/ru/syslog.html)
  - [English](http://www.bsdstore.ru/en/syslog.html)
  - [Deutsch](http://www.bsdstore.ru/de/syslog.html)

2020-10 upd: we reached the first fundraising goal and rented a server in Hetzner for development! Thank you for [donating](https://www.patreon.com/clonos) !

Attention! Current pages describe **CBSD** version **13.0.x**. If you are using an older version, please update first.

Attention! I apologize for the automatic translation of this text. You can improve it by sending me a more correct version of the text or fix html pages via [GITHUB repository](https://github.com/cbsd/cbsd-wwwdoc).

# FreeBSD: syslog and debugging

## syslog

Starting from **CBSD** 11.1.19 You can use the syslog subsystem to collect messages that occur during the process of **CBSD** scripts

The configuration file for the log subsystem: _~cbsd/etc/defaults/logger.conf_. Create new file _~cbds/etc/logger.conf_ to override the default values.

Using the syslog configuration file, you can redirect all messages of **CBSD** in a separate file and in the future use different solutions for the collection and analysis of messages.

_/etc/syslog.d/cbsd.conf_:

```
!cbsd
*.*                     /var/log/cbsd.log

```

And create empty file:

```
touch /var/log/cbsd.log

```

After syslog restarting, messages from **CBSD** can be read in a file /var/log/cbsd.log

## debugging

If you encounter an error in the script, you can get a trace of all sh commands executed by running a particular **CBSD** script through the **CBSD\_DEBUG** environment variable, for example:

```
env CBSD_DEBUG=1 cbsd jls

```

Copyright © 2013—2024 CBSD Team.

