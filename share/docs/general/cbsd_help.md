# **CBSD** Built-in help

## Getting help

The CBSD is a modular system and, depending on the configuration, may have a different set of subcommands.
To get a list of commands, use:

```
cbsd help
```

The list of commands is dynamic and is compiled during the `cbsd initenv` operation.

![cbsd help](https://convectix.com/img/cbsd-help1.png)


CBSD strives to be a self-documenting system, so as a rule, for each command you can get a small reference through
```
cbsd CMD --help
```
or:
```
cbsd help CMD
```
Therefore, make it a rule to first read the built-in help and only then refer to external documentation.

![cbsd cmd help](https://convectix.com/img/cbsd-help3.png)

Commands are divided into categories (called modules). You can filter only those commands that are related to a particular module. 
For example, display commands related to the module 'qemu':
```
cbsd help module=qemu
```
