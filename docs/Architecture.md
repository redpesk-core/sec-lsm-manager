## Architecture

sec-lsm-manager has the following architecture :

<div align="center">
<img src="./images/sec-lsm-manager.png" alt="architecture">
</div>

### sec-lsm-managerd, sec-lsm-manager-smackd, sec-lsm-manager-selinuxd

sec-lsm-managerd, sec-lsm-manager-smackd and sec-lsm-manager-selinuxd are the main components of the project.
The sec-lsm-managerd binary will launch one of the other two depending on the mandatory access control.

It is possible to install sec-lsm-manager-smackd or sec-lsm-manager-selinuxd or both.
They are those that will define the security policy, label files and send
permissions to cynagora.

To receive the instructions, they will create a socket and listen it.
The socket can be a systemd or unix socket.

### libsec-lsm-manager

libsec-lsm-manager is a shared library that will allow to communicate with the daemon.

It is necessary to include the `sec-lsm-manager.h` file to use it.

### sec-lsm-manager-cmd

sec-lsm-manager-cmd is a utility that allows to use the shared library via the command line.
