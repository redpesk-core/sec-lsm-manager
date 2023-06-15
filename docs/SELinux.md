# SELinux

## Basics

### Policy Module

A policy module will define your application rules and it is easy to install and uninstall.

It contains three files :

- Type Enforcement (.te)

- File Context (.fc)

- Interfaces (.if)


#### Type Enforcement files

Type enforcement files contains the rules to confine your application


#### File Context files

File contexts files define which label to give to each file


#### Interface files

Interface files contain interfaces to says how they interact with the modules

### Compile

To compile a SELinux module, we place our different files in a folder.

```bash
make -f /usr/share/selinux/devel/Makefile -C /usr/share/sec-lsm-manager/selinux-rules demo-app.pp
```

### Install / Uninstall

To install the newly created SELinux module, use the following command :

```bash
semodule -i demo-app.pp
```

And to remove it :

```bash
semodule -r demo-app.pp
```

## Sources

Vermeulen, S. (2015). *Selinux Cookbook*. Packt Publishing.

https://mgrepl.fedorapeople.org/PolicyCourse/writingSELinuxpolicy_MUNI.pdf

https://debian-handbook.info/browse/fr-FR/stable/sect.selinux.html

