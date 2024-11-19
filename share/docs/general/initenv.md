## initenv

### Initial setup

To initialize the main working copy of **CBSD** we need to run **initenv**, while setting the environment variable **workdir** to the location of the working directory. 
In some cases, you can initialize several independent working directories (for example, different datapools) and work with them via an environment variable.

We then answer a series of questions to complete the configuration. 
The file system for **CBSD** (or rather, the directory **jails-data** in it) needs to be large enough to accommodate the containers/VM data. 
If you only use one working directory, please ensure it is set as the home directory for **cbsd** user (in this case the working directory in /usr/jails):

Initialization with the working catalogue into _/usr/jails_ (default):

```
env workdir="/usr/jails" /usr/local/cbsd/sudoexec/initenv
```

Attention! If you are using a ZFS-based platform, before initialization it is highly recommended to create a separate dataset for the CBSD working directory in order to avoid possible conflicts and influences when operating various
automatic snapshot systems (and also to facilitate the migration of the CBSD working directory to other servers), for example:

```
# zfs create zroot/jails
# zfs set mountpoint=/usr/jails zroot/jails

```

The start dialogue will appear with questions on how to configure **CBSD**. 
Pressing Enter with no entered values will set system defaults. 
Once done updating **CBSD** it is necessary to run initenv again. 
After the first run of initenv it can be started through **CBSD** without specifying the **env** variable again and the working dir variable will be stored in _/etc/rc.conf_.

On first initialization the following questions will be asked:

- **Please fill nodename** — Node name. If you plan on working with several nodes, this name should be unique. It is recommended to use a name, or full hostname (including domain). This name is used when working with remote hosts. Example: node1.my.domain.
- **Please fill nodeip** — Working and static IP address of a node. It shouldn't be an alias and it is desirable to register this IP on a separate interface (free from any other except a management-traffic). For example: 192.168.1.2
- **Please fill jnameserver** — List of IP DNS servers to be inserted in the newly created jail's _/etc/resolv.conf_. If several addresses are added, they must be separated by commas. It is recommended to setup the master-node server for caching DNS queries.
- **Please fill nodeippool** — The list of subnets in which jails may be started. If adding more than one network use space as delimer. For example: 10.0.0.0/24 192.168.1.128/29
- **Please fill natip** — This specivies the IP address which will represent itself as NAT for private addresses. Usually it is IP of your node. For example: 192.168.1.2
- **Please fill fbsdrepo** — Whether or not to use an official FreeBSD repository for base/templates. Expected answers are 1 or 2 . If on official FreeBSD servers base it is not revealed — **CBSD** repository is used. For example: 1.
- **Please fill zfsfeat** — Whether to use features of ZFS (clones, snapshots). The answer 1 or 2 is expected. The question won't be asked, if the host system is started not on ZFS.
- **Configure NAT for RFC1918 Network?** — Whether to use network address translation (NAT) for private addresses. When jails are created in RFC1918 networks, it is necessary to enable this for internet access. For example: 1.
- **Which one NAT framework should be use: \[pf, ipfw, nft, ipfilter\]**\- What tool for NAT to prefer. Recommended — 'pf' for FreeBSD, 'nft' for Linux. For example: pf

### initenv preseed (auto)

You can use non-interactive initialization of **CBSD** by using usual ASCII file with pre-prepared answers to questions.

![initenv1.png](https://convectix.com/img/initenv1.png?raw=true)

This can be useful if you have to do this often, in large quantities, or you embed **CBSD** into your existing infrastructure to automatically
deploy and configure environments.

As an example, you can see (and/or use) the _/usr/local/cbsd/share/initenv.conf_ file. For initialization, it is enough to specify the path to the file with answers:

```
/usr/local/cbsd/sudoexec/initenv /usr/local/cbsd/share/initenv.conf
```

Moreover, you can use a configuration file and override some parameters via args:
```
/usr/local/cbsd/sudoexec/initenv /usr/local/cbsd/share/initenv.conf default_vs=1
```

**Info:** These settings can be changed at a later time via the command **cbsd initenv-tui** or in a file _${workdir}/var/db/local.sqlite_, which is a SQLite3 database.

![initenv-tui1.png](https://convectix.com/img/initenv-tui1.png?raw=true)

### initenv hooks

When the initenv command is executed (it must be executed each time when CBSD is updated), various modifications and migrations are performed.

You can embed in this process your own scripts and scenatio that will be executed **before** any modifications, or **after**.

This can be useful, for example, for backup scenarios (or evacuating environments to a neighboring node) as a preliminary hook and notification for
monitoring or another communication channel about a successful update as post-hook.

To do this, create in the workdir a directory named **upgrade**:

```
mkdir -p ~cbsd/upgrade
```

Any scripts that start with _pre-initenv-_ or _post-initenv-_ and have an executable flag will be executed
before modifying initenv or after, respectively.
