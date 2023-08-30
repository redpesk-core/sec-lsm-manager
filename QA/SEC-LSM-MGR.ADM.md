# Administration manual of redpesk-core/sec-lsm-manager

## Content

This manual describes everything an administrator or an architect
should know about the security manager of redpesk, below designated
by SEC-LSM-MANAGER.

SEC-LSM-MANAGER installation is made of the following files:

```
/usr/lib/systemd/system/sec-lsm-manager.service
/usr/lib/systemd/system/sec-lsm-manager.socket
/usr/lib/systemd/system/sec-lsm-manager.target
/usr/lib/systemd/system/sockets.target.wants
/usr/lib/systemd/system/sockets.target.wants/sec-lsm-manager.socket
/usr/lib/libsec-lsm-manager.so.2.3.1
/usr/lib/libsec-lsm-manager.so.2
/usr/bin/sec-lsm-manager-selinux
/usr/bin/sec-lsm-manager-smackd
/usr/bin/sec-lsm-managerd
/usr/bin/sec-lsm-manager-cmd
/usr/share/sec-lsm-manager/app-template.selinux
/usr/share/sec-lsm-manager/app-template.smack
```

The SEC-LSM-MANAGER comes with two flavours, depending on the
targetted LSM: Smack or SELinux.

Because of it, SEC-LSM-MANAGER comes with a launcher whose name is
`sec-lsm-managerd` and two effectives binaries: `sec-lsm-managerd-smack`
and `sec-lsm-managerd-selinux`. The launcher take care of discovering the
effective binary to be run and starts it.


## Starting the service

### Start on need

The service SEC-LSM-MANAGER should not normally be running.
It is started on need.


## Command line arguments

Here is the output of invoking `sec-lsm-manager-smackd` with arguments `--help`

```
$ sec-lsm-manager-smackd --help

usage: sec-lsm-managerd [options]...

otpions:
    -u, --user xxx        set the user
    -g, --group xxx       set the group
    -G  --groups xxx,yyy  set additional groups
    -l, --log             activate log of transactions
    -k, --keep-going      continue to run on some errors
    -s, --shutoff VALUE   shutting off time in seconds

    -S, --socketdir xxx   set the base directory xxx for sockets
                            (default: /home/jobol/.locenv/sec/var/run)
    -M, --make-socket-dir make the socket directory
    -O, --own-socket-dir  set user and group on socket directory

    -h, --help            print this help and exit
    -v, --version         print the version and exit
```


### Arguments for help and version

The argument `--help` shows the usage of the program and exits.
See above for an example of output.

The argument `--version` shows the version of the program.

### Arguments for global settings

The argument `--log` turns on the reporting of communications
to the logger. This is a spying tip intended for debugging.
This can be changed dynamically using commend `log`.

The argument `--keep-going` tells the server to not stop
when it reaches errors that otherwise would stop it.
This is useful for debugging or running unprivileged,
in the former case, privileged accesses fail and are logged.

The argument `--shutoff` `VALUE` allows to set the duration
of inactivity leading to a shutdown of the service.
That duration is set in seconds. Setting a negative value
is possible, it tells to disable auto shutdown.

The security manager being used when installing and
uninstalling applications, it is not generally used during
normal operations and so it is safer and cleaner to stop
it after some time of inactivity.


### Arguments for setting service credentials

The argument `--user` `USER` tells SEC-LSM-MANAGER to be the user
of the service. The given value, `USER`, can be given either using
its numeric identifier or its user name.

The argument `--group` `GROUP` tells SEC-LSM-MANAGER to be the user
of the service. The given value, `GROUP`, can be given either using
its numeric identifier or its group name.

The argument `--groups` `GROUP1,GROUP2,...` tells SEC-LSM-MANAGER to
to set the supplementaries group to the given list of groups.
The groups given in the list can be either numeric or symbolic name.
Groups are separated by commas and/or spaces.

When the argument `--user` is given using a symbolic name but the group
is not given, then the default group of the given user is used.

If any of these options is not given, the SEC-LSM-MANAGER doesn't not
change the corresponding attribute.


### Arguments for setting socket

The argument `--socketdir` `DIRECTORY` tells the location where to create
the communication socket.

The argument `--make-socket-dir` if present tells to create the directory
where will be created the socket. If not present, the directory is not created.

The argument `--own-socket-dir` if present and if the directory of the
socket is created tells to set the user and group of the created directories
using the user and group set using options `--user` and `--group`.
Otherwise, the directories are created using the current credentials.


### SEC-LSM-MGR.ADM-CAP

List of capabilities required by SEC-LSM-MANAGER

- `CAP\_MAC\_ADMIN`
- `CAP\_DAC\_OVERRIDE`
- `CAP\_MAC\_OVERRIDE`
- `CAP\_SYS\_ADMIN`
- `CAP\_DAC\_READ\_SEARCH`
- `CAP\_SETFCAP`
- `CAP\_FOWNER`

See the developper manual for analysis of why
these capabilities are required.


### SEC-LSM-MGR.ADM-USRGRP

user: sec-lsm-manager
group: sec-lsm-manager
supplementatry groups: cynagora

## Template files

SEC-LSM-MANAGER needs templating files that describes how to
compute security rules from files types and application names.

The templating files are located in the directory `/usr/share/sec-lsm-manager`.

## manual interaction with SEC-LSM-MANAGER

It is possible to interact with the security manager if you have root
access to some console.

All the interaction using either `sec-lsm-manager-cmd` or `socat`
are mostly intended for debuging and should not happen in normal use.

### using sec-lsm-manager-cmd

The command `sec-lsm-manager-cmd` can be used. That command is
interactive and self documented, so launch it and ask for help.


### using socat

If you master the [protocol](protocol.md), it is possible to use the "swiss army knife"
tool `socat` as shows the below example:

```
$ socat stdio unix-client:/var/run/sec-lsm-manager.socket
sec-lsm-manager 1
done 1
log
done off
log on
done on
```

