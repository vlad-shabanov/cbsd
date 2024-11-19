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
- Lang »  - [Русский](http://www.bsdstore.ru/ru/13.0.x/wf_expose_ssi.html)
  - [English](http://www.bsdstore.ru/en/13.0.x/wf_expose_ssi.html)
  - [Deutsch](http://www.bsdstore.ru/de/13.0.x/wf_expose_ssi.html)

2020-10 upd: we reached the first fundraising goal and rented a server in Hetzner for development! Thank you for [donating](https://www.patreon.com/clonos) !

Attention! Current pages describe **CBSD** version **13.0.x**. If you are using an older version, please update first.

Attention! I apologize for the automatic translation of this text. You can improve it by sending me a more correct version of the text or fix html pages via [GITHUB repository](https://github.com/cbsd/cbsd-wwwdoc).

# expose: tcp/udp port forwarding from master host to jail

## command: expose

```
			% cbsd expose jname=test2 mode=add in=200 out=200
			% cbsd expose jname=test2 mode=delete in=200 out=200
			% cbsd expose jname=test2 mode=list
			% cbsd expose jname=test2 mode=clear
			% cbsd expose jname=test2 mode=flush

```

By command **cbsd expose** you can create forward rule for tcp/udp port from external IP to jail.

Copyright © 2013—2024 CBSD Team.

