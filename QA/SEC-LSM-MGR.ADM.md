# Manual of redpesk-core/sec-lsm-manager


.VERSION: DRAFT

.AUTHOR: José Bollo [IoT.bzh]

.AUDIENCE: ENGINEERING

.DIFFUSION: CONFIDENTIAL

.git-id($Id$)

This manual describes everything an administrator or an architect
should know about the security manager of redpesk, below designated
by SEC-LSM-MANAGER.

## Content

The SEC-LSM-MANAGER comes in two flavours: Smack and SELinux.
Because some files are common to the two flavours, the SEC-LSM-MANAGER
is delivered through 3 main RPMs:

- sec-lsm-manager: common files
- sec-lsm-manager-smack: Smack soecific files
- sec-lsm-manager-selinux: SELinux specific files

At least two of them are required:

- for Smack, sec-lsm-manager and sec-lsm-manager-smack
- for SELinux, sec-lsm-manager and sec-lsm-manager-selinux

It is possible but not recommended to install the 3 RPMs, and to
switch at boot time to one expected flavour. In that case the
SEC-LSM-MANAGER will chose the required implementation for the
currently active security module.

The search of the available RPMs using DNF gives the below results:

```
# dnf search sec-lsm-manager
RedPesk Middleware arz-1.1 Update - x86_64      212 kB/s | 132 kB     00:00    
RedPesk Baseos arz-1.1 Update - x86_64          3.9 MB/s | 6.2 MB     00:01    
RedPesk Config                                  1.9 kB/s | 903  B     00:00    
=================== Name & Summary Matched: sec-lsm-manager ====================
sec-lsm-manager.x86_64 : sec-lsm-manager service (SMACK, SELinux)
sec-lsm-manager-tool.x86_64 : Tiny tool for sec-lsm-manager
sec-lsm-manager-devel.x86_64 : Development libraries and header files for sec-lsm-manager
sec-lsm-manager-selinux.x86_64 : sec-lsm-manager service for SELinux
sec-lsm-manager-selinux-redtest.x86_64 : Test package of sec-lsm-manager service for SELinux
sec-lsm-manager-smack.x86_64 : sec-lsm-manager service for SMACK
sec-lsm-manager-smack-redtest.x86_64 : Test package of sec-lsm-manager service for SMACK
```

This shows 4 new optional RPMs:

- sec-lsm-manager-tool: The tool sec-lsm-manager-cmd
- sec-lsm-manager-devel: Header and library for developping a client
- sec-lsm-manager-smack-redtest: Unit tests for Smack
- sec-lsm-manager-selinux-redtest: Unit tests for SELinux

These new RPMs are for development and for testing.

### Content of the RPM sec-lsm-manager

Below the ouput of listing items of the RPM sec-lsm-manager.

```
[root@localhost ~]# rpm -ql sec-lsm-manager
/usr/bin/sec-lsm-managerd
/usr/lib/.build-id/dd/e90682ded53412aaec2a52e87dff7972419630
/usr/lib/.build-id/f1/4113a77ae6745054fd26eb264921c73f7558ef
/usr/lib/systemd/system/sec-lsm-manager.service
/usr/lib/systemd/system/sec-lsm-manager.socket
/usr/lib/systemd/system/sec-lsm-manager.target
/usr/lib/systemd/system/sockets.target.wants
/usr/lib/systemd/system/sockets.target.wants/sec-lsm-manager.socket
```

Beside the `build-id` files, it contains the services files for systemd
and the main binary switcher that checks the currently active LSM,
Smack or SELinux, for running the correctly favoured SEC-LSM-MANAGER.

### Content of the RPM sec-lsm-manager-smack

Below the ouput of listing items of the RPM sec-lsm-manager-smack.

```
[root@localhost ~]# rpm -ql sec-lsm-manager-smack
/usr/bin/sec-lsm-manager-smackd
/usr/lib/.build-id/0d/496c1e91f597182e6a125fb016ad99db88d851
/usr/share/sec-lsm-manager/app-template.smack
```

Beside the `build-id` files, it contains the Smack flavoured
SEC-LSM-MANAGER binary: `sec-lsm-manager-smackd` and its template
file.


### Content of the RPM sec-lsm-manager-selinux

Below the ouput of listing items of the RPM sec-lsm-manager-selinux.

```
[root@localhost ~]# rpm -ql sec-lsm-manager-selinux
/usr/bin/sec-lsm-manager-selinuxd
/usr/lib/.build-id/db/c076b080d501915c5e3cdea24fa16996fde244
/usr/share/sec-lsm-manager/app-template.te
/usr/share/sec-lsm-manager/app-template.if
/usr/share/sec-lsm-manager/script/build-module.sh
```

Beside the `build-id` files, it contains the Smack flavoured
SEC-LSM-MANAGER binary: `sec-lsm-manager-selinuxd` and its template
file.


## Checklist

### The user and group sec-lsm-manager exist

The user and the group `sec-lsm-manager` must exist and be
unique.


The command for checking that the user `sec-lsm-manager` exists
and is unique:

```
# grep sec-lsm-manager /etc/passwd
sec-lsm-manager:x:991:991::/var/lib/empty:/bin/false
# grep :x:991: /etc/passwd
sec-lsm-manager:x:991:991::/var/lib/empty:/bin/false
```

Same for the group `sec-lsm-manager`:

```
# grep sec-lsm-manager /etc/group
sec-lsm-manager:x:991:
# grep :x:991: /etc/group
sec-lsm-manager:x:991:
```

### The user sec-lsm-manager can't login

The login shell of the user `sec-lsm-manager` must be `/bin/false`.

```
# grep sec-lsm-manager /etc/passwd
sec-lsm-manager:x:991:991::/var/lib/empty:/bin/false
```

### Only required RPM are installed

On the target run the command `dnf list --installed | grep sec-lsm-manager`.

Only 2 rpm should be installed

- `sec-lsm-manager` and `sec-lsm-manager-smack` on a smack system
- `sec-lsm-manager` and `sec-lsm-manager-sselinux` on a SELinux system

### Check the systemd files

The installed systemd files are:

- /usr/lib/systemd/system/sec-lsm-manager.service
- /usr/lib/systemd/system/sec-lsm-manager.socket
- /usr/lib/systemd/system/sec-lsm-manager.target
- /usr/lib/systemd/system/sockets.target.wants/sec-lsm-manager.socket

Listing it on the system, using command `ls -lZ` must show the below result (except the date):

```
-rw-r--r-- 1 root root _ 465 Oct 24 16:40 /usr/lib/systemd/system/sec-lsm-manager.service
-rw-r--r-- 1 root root _ 373 Oct 24 16:40 /usr/lib/systemd/system/sec-lsm-manager.socket
-rw-r--r-- 1 root root _  68 Oct 24 16:21 /usr/lib/systemd/system/sec-lsm-manager.target
lrwxrwxrwx 1 root root _  25 Oct 24 16:40 /usr/lib/systemd/system/sockets.target.wants/sec-lsm-manager.socket -> ../sec-lsm-manager.socket

```

And their SHA256 are

```
ea629ead284cf23b9530324e063577a195d608cec62d4a270ecdbb2d87106a20  /usr/lib/systemd/system/sec-lsm-manager.service
df7a8cacf13e3fcd3d0474c543d164a55ba653355841f8869eeb2506df7c7a59  /usr/lib/systemd/system/sec-lsm-manager.socket
9905c23762744d00d39c69aedc0e1c592e8495bffd20f14be08dd248bee2cad1  /usr/lib/systemd/system/sec-lsm-manager.target
```

The content of the service file is:

```
[Unit]
Description=Security Manager service

[Service]
Type=notify
ExecStart=/usr/bin/sec-lsm-managerd --user sec-lsm-manager --group sec-lsm-manager --groups cynagora
Restart=on-failure

KillMode=process
TimeoutStopSec=3

Sockets=sec-lsm-manager.socket

CapabilityBoundingSet=CAP_MAC_ADMIN CAP_DAC_OVERRIDE CAP_MAC_OVERRIDE CAP_SYS_ADMIN  CAP_DAC_READ_SEARCH CAP_FOWNER CAP_SETFCAP CAP_SETUID CAP_SETGID
#NoNewPrivileges=true

[Install]
WantedBy=multi-user.target
```

The content of the socket file is:

```
[Unit]
Description=sec-lsm-manager socket

[Socket]
FileDescriptorName=sec-lsm-manager
ListenStream=/var/run/sec-lsm-manager.socket
SocketUser=sec-lsm-manager
SocketGroup=sec-lsm-manager
SocketMode=0660
SmackLabelIPIn=@
SmackLabelIPOut=@

Service=sec-lsm-manager.service

[Unit]
Wants=sec-lsm-manager.target
Before=sec-lsm-manager.target

[Install]
WantedBy=sockets.target
```

The content of the target file is:

```
[Unit]
Description=sec-lsm-manager target
DefaultDependencies=true
```


### Check the smack files

The RPM sec-lsm-manager-smack installs one template file:
`/usr/share/sec-lsm-manager/app-template.smack`.

Listing it using `ls -lZ should show:

```
-rw-r--r-- 1 sec-lsm-manager sec-lsm-manager _ 555 Oct 24 16:40 /usr/share/sec-lsm-manager/app-template.smack
```

Its SHA256 sum should be:

```
1124e3ed9077ac4628621f2d697420322a666215467d302f52df3591ef852205  /usr/share/sec-lsm-manager/app-template.smack
```

And its contents:

```
System App:{{id}} rwxa
App:{{id}} System:Shared rx
App:{{id}} User:App-Shared rwx
App:{{id}} System wx
App:{{id}} App:{{id}}:Lib rx
App:{{id}} App:{{id}}:Conf rx
App:{{id}} App:{{id}}:Http rx
App:{{id}} App:{{id}}:Data rx
App:{{id}} App:{{id}}:Exec rx
App:{{id}} User:Home rx
{{#has-plugs}}
App:{{id}} App:{{id}}:Plug rx
{{/has-plugs}}
{{#plugs}}
App:{{impid}} App:{{id}}:Plug rx
App:{{impid}} App:{{id}}:Lib rx
App:{{impid}} App:{{id}}:Exec rx
App:{{impid}} App:{{id}}:Data rx
App:{{impid}} App:{{id}}:Conf rx
App:{{impid}} App:{{id}}:Http rx
{{/plugs}}
```


## Managing the service

### Start on need

The service SEC-LSM-MANAGER should not normally be running.
It is started on need and it stops after a period of inactivity.

This is the normal behaviour and there is no known reason to
launch the service manually.

Though, the context of the service is manageable using
invocation arguments. The default settings, as described above,
are safe.

### Command line arguments

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


