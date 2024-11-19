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
- Lang »  - [Русский](http://www.bsdstore.ru/ru/racct_exports.html)
  - [English](http://www.bsdstore.ru/en/racct_exports.html)
  - [Deutsch](http://www.bsdstore.ru/de/racct_exports.html)

2020-10 upd: we reached the first fundraising goal and rented a server in Hetzner for development! Thank you for [donating](https://www.patreon.com/clonos) !

Attention! I apologize for the automatic translation of this text. You can improve it by sending me a more correct version of the text or fix html pages via [GITHUB repository](https://github.com/cbsd/cbsd-wwwdoc).

Attention! Current pages describe **CBSD** version **13.0.x**. If you are using an older version, please update first.

# CBSD: export RACCT metrics

## Intro

wip/draft

Export metrics to beanstalk highly experimental and not enabled by default. So no, beanstalkd at the moment is optional ( and probably will remain so ). Another backend for metrics is SQLite3 (already done) and Prometheus ( still wip ).


If you with to play with metrics enable cbsd racct services:

- for bhyve metrics:




```
sysrc cbsd_statsd_bhyve_enable=YES
```







```
service cbsd-statsd-bhyve start
```

- for jail metics:




```
sysrc cbsd_statsd_jail_enable=YES
```







```
service cbsd-statsd-jail start
```

- for hoster metics ( **required by DRS**):




```
sysrc cbsd_statsd_hoster_enable=YES
```







```
service cbsd-statsd-hoster start
```


Also you need to enable/install beanstalkd:

```
pkg install -y net/beanstalkd
```

```
sysrc beanstalkd_enable=YES
```

```
sysrc beanstalkd_flags="-l 127.0.0.1 -p 11300"
```

```
service beanstalkd start
```

To receive real-time metrics use 'racct-jail' tube in beasntalkd to receive metrics for jail (in json), 'racct-bhyve' tube for bhyve and 'racct-system' for hoster metrics

update rate and other settings are in the corresponding configuration file in _~cbsd/etc/default_ directory ( **racct-{jail,bhyve,hoster}-statsd.conf**). Please use _~cbsd/etc_ directory
for overwrite default settings.

E.g. simple python-based client:

```
pkg install -y net/py-beanstalkc   (install client)
```

```
#!/usr/bin/env python2.7

import beanstalkc
#import time
import sys

try:
    bsk = beanstalkc.Connection(host='localhost', port=11300)
    bsk.watch('racct-jail')
    bsk.use('racct-jail')
except:
    print "cannot open connection"
    sys.stdout.flush()
    sys.exit(1)

while 1:
    #print time.strftime('%Y-%m-%d %X'), " waiting for work ..."
    sys.stdout.flush()
    job = bsk.reserve(timeout=60)
    if job is not None:
        # I had to delete it immediately
        job.delete()
        try:
            sys.stdout.flush()
            print(repr(job.body))
        except:
            print "bad things"
            sys.stdout.flush()

```

Copyright © 2013—2024 CBSD Team.

