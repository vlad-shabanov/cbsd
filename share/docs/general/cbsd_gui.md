# CBSD GUI interface

*CBSD* is a set of low-level operations for working with virtual environments (creating a virtual machine/container, 
adding a disk, adding a network card, etc.) and is primarily focused on the text interface. 

However, for those who prefer not to delve into a large amount of information below and who prefer to work in graphical environments, 
this information may be useful.

## Web based interface

_This work was sponsored by the project donor fund_

**CBSD** offers a web based interface called **ClonOS**.
**ClonOS** is developed independently to be a standalone
platform to manage and create virtual environments in FreeBSD. **ClonOS**
itself is also FreeBSD based, and can be installed and run as a purpose
built FreeBSD distribution.


More information can be found on the **ClonOS** website:
[clonos.convectix.com](https://clonos.convectix.com)

Once enough positive feedback about **ClonOS** is received,
and the developers feel it stable enough, it will be available via the
FreeBSD ports tree.


While **ClonOS** is not currently considered to be production
ready, the following features are functionally stable;

- Interface to create and manage jails
- Interface for manage bhyve
- Interface to CBSD forms (Helpers to assist with common tasks)
- Interface to work with/configure FreeBSD sources and bases

![ClonOS-jail](https://clonos.convectix.com/img/clonos_beta1_dark.png?raw=true)
![ClonOS-VM](https://clonos.convectix.com/img/clonos_alpha2.png?raw=true)

## QT6-based interface

_This work was sponsored by the project donor fund_

Cross-platform QT6-based GUI client for Mobile phone and Desktop PC:

[MyBee-QT](https://github.com/myb-project/mybee-qt)

![MyBee-QT1](https://convectix.com/img/mybee-qt1.png?raw=true)
![MyBee-QT2](https://convectix.com/img/mybee-qt2.png?raw=true)
![MyBee-QT3](https://convectix.com/img/mybee-qt3.png?raw=true)

## (alternative) NCURSES-based interface (3rd party)

Some people are not satisfied with dialog forms and prefer an alternative NCURSES interface, e.g.: [cbsd-tui](https://github.com/Peter2121/cbsd-tui)

![CBSD-TUI](https://convectix.com/img/cbsd-tui1.png?raw=true)
