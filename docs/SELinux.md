# SELinux

## Basics

### Policy module

A policy module will define your application rules. It is easy to install and uninstall.

It contains three files :

- type enforcement (.te): contains the rules to confine your application.
- file context (.fc): defines which label to give to each file.
- interfaces (.if): contains interfaces to say how applications interact with the modules

### Compile

To compile a SELinux module, we place our different files in a folder.
The output is a policy package (.pp) file.

Package requirement: `selinux-policy-dev` for Debian, `selinux-policy-devel` for RHEL

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

- Vermeulen, S. (2015). *Selinux Cookbook*. Packt Publishing.
- <https://mgrepl.fedorapeople.org/PolicyCourse/writingSELinuxpolicy_MUNI.pdf>
- <https://debian-handbook.info/browse/fr-FR/stable/sect.selinux.html>
