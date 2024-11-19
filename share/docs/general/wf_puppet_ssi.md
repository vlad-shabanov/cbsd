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
- Lang »  - [Русский](http://www.bsdstore.ru/ru/13.0.x/wf_puppet_ssi.html)
  - [English](http://www.bsdstore.ru/en/13.0.x/wf_puppet_ssi.html)
  - [Deutsch](http://www.bsdstore.ru/de/13.0.x/wf_puppet_ssi.html)

2020-10 upd: we reached the first fundraising goal and rented a server in Hetzner for development! Thank you for [donating](https://www.patreon.com/clonos) !

Attention! Current pages describe **CBSD** version **13.0.x**. If you are using an older version, please update first.

Attention! I apologize for the automatic translation of this text. You can improve it by sending me a more correct version of the text or fix html pages via [GITHUB repository](https://github.com/cbsd/cbsd-wwwdoc).

# Work with CBSD through Puppet

When you operate a large number of nodes and containers, handmade container management becomes ineffective

There are many of today's popular configuration management systems: **Ansible**, **Chef**, **Saltstack**, **Puppet** etc., written in different languages and with different concepts and principles of.

At this point, we support CBSD module for **Puppet** and here is a brief information on its use.

Project page at **Puppet Forge**: [CBSD puppet](https://forge.puppet.com/olevole/cbsd)

## Description - for which this module

Puppet CBSD

module allows to write server configuration using declarative programming (Ruby DSL, domain-specific language).

In other words, you create a manifesto describing what the container and with what parameters and characteristics should be on a particular server.

Further work on the implementation of these requirements **Puppet** takes over.

## Examples of the use of the module

For installing **CBSD** on the server, just declare the class _cbsd_:

```
		class { 'cbsd': }

```

If you are installing **CBSD** manually (eg not via pkg/ports, and have Git version, you can disable installation/registration CBSD package by module:

```
	class { 'cbsd':
		manage_repo => false,
	}

```

In the class of all parameters can be defined initenv. For example, to initialize **CBSD** with _/usr/jails_ workdir and enable **NAT** framework:

```
	class { 'cbsd':
		jnameserver => "8.8.8.8",
		nat_enable => '1',
		defaults => {
			'workdir'         => '/usr/jails',
		}
	}

```

You can force **CBSD** download specific base version:

```
	class { "cbsd::freebsd_bases":
		ver => [ '12' ],
		stable => 1,
	}

```

Create jail:

```
	cbsd::jail { 'myjail0':
		pkg_bootstrap => '0',
		host_hostname => 'myjail0.my.domain',
	}

```

If you prefer to work with parameters through **Yaml**, в **Hierra** it might look like:

```
	cbsd::jails:
	  myjail0:
	    host_hostname: 'myjail0.my.domain'

```

Copyright © 2013—2024 CBSD Team.

