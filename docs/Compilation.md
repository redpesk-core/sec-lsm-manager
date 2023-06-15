# Compilation

## Dependencies

> If you are missing any dependencies please follow this guide : [redpesk-build-host]({% chapter_link host-configuration-doc.setup-your-build-host %})

### Fedora / Centos

```
dnf install check-devel sec-cynagora-devel libsemanage-devel libselinux-devel systemd-devel libcap-devel libsmack-userspace-devel
```

### Ubuntu / Debian

```
apt install check sec-cynagora-dev libsemanage1-dev libselinux1-dev libsystemd-dev libcap-dev
```

## Options

It is possible to modify the following compilation options when executing the cmake command:

- WITH\_SYSTEMD (default : ON) : systemd socket activation
- WITH\_SMACK (default : OFF)  : SMACK mode
- WITH\_SELINUX (default : OFF) : SELinux mode

- WITH\_SIMULATION (default : OFF) : active simulations for cynagora, SMACK and SELinux
- SIMULATE\_CYNAGORA (default : OFF) : simulate cynagora
- SIMULATE\_SMACK (default : OFF) : simulate SMACK
- SIMULATE\_SELINUX (default : OFF) : simulate SELinux

- FORTIFY (default : ON) : fortify source code
- COMPILE\_TEST (default : ON) : compile tests
- DEBUG (default : OFF) : active debug mode (symbols, debug message)

For example with DEBUG option and only SELinux :

```bash
cmake -DDEBUG=ON -DWITH_SELINUX=ON ..
```


## Environment Variables

Then there are variables defined at compile time in the `CMakeLists.txt` file that can be changed at runtime by defining an environment variable.

For example the SEC\_LSM\_MANAGER\_SOCKET\_NAME variable which contains `sec-lsm-manager.socket` can be modified at launch :

```bash
export SEC_LSM_MANAGER_SOCKET_NAME="new-socket-name.socket"
/usr/bin/sec-lsm-managerd
```

It is possible to modify the following environment variables:

- SELINUX\_RULES\_DIR (default : "/usr/share/sec-lsm-manager/selinux-rules")
- SELINUX\_MAKEFILE (default : "/usr/share/selinux/devel/Makefile")
- SEC\_LSM\_MANAGER\_DATADIR (default : "/usr/share/sec-lsm-manager")
- SEC\_LSM\_MANAGER\_SOCKET\_NAME (default : "sec-lsm-manager.socket")

- COMPILE\_SCRIPT\_DIR (default : "/usr/share/sec-lsm-manager/script")
- COMPILE\_SCRIPT\_NAME (default : "build-module.sh")

- TE\_TEMPLATE\_FILE (default : "app-template.te")
- IF\_TEMPLATE\_FILE (default : "app-template.if")
- TEMPLATE\_FILE (default : "app-template.smack")

- SELINUX\_FS\_PATH (default : "/sys/fs/selinux")
- SMACK\_FS\_PATH (default : "/sys/fs/smackfs")

- SMACK\_POLICY\_DIR (default : "/etc/smack/accesses.d", simulation : "/usr/share/sec-lsm-manager/smack-simulation")
- SELINUX\_POLICY\_DIR (simulation : "/usr/share/sec-lsm-manager/selinux-simulation")

