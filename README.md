# SEC-LSM-MANAGER

```
                                                                                       systemd-socket
                                                                                              ^
                       +---------------------------+            +----------------------+      |             +-----------------------------+
                       |                           |            |                      |      |             |                             |
                       |  libsec-lsm-manager-core  +<-----------+   sec-lsm-managerd   +<-----+-------------+  libsec-lsm-manager-client  |
                       |                           |            |                      |                    |                             |
                       +--------+----------------+-+            +----------------------+                    +-----------------------+-----+
                                |                |                                                                                  ^
                                |                |                                                                                  |
                                |                |                                                                                  |
      systemd-socket  <---------+                |                                                                                  |
                                |                |                                                                                  |
                                v                v                                                                           +------+----------------+
+-------------------------------+---+  +---------+------------------------------------------------------+                    |                       |
|                                   |  |                                                                |                    | sec-lsm-manager-cmd   |
|             CYNAGORA              |  | +----------------------------+ +-----------------------------+ |                    |                       |
|                                   |  | |                            | |                             | |                    | id redpesk-service-id |
|                                   |  | |          SMACK             | |         SELINUX             | |                    |                       |
|                                   |  | |                            | |                             | |                    |                       |
|     id  * * perm123 yes forever   |  | |                            | |     install semodule        | |                    | path /tmp/toto tmp    |
|                                   |  | |   /etc/smack/accesses.d/   | |                             | |                    |                       |
|                                   |  | |                            | |  ==> redpesk-service-id     | |                    |                       |
|                                   |  | |                            | |                             | |                    | permission perm123    |
|                                   |  | |  app-redpesk-service-id1   | |   +--------------------+    | |                    |                       |
|                                   |  | |                            | |                             | |                    |                       |
|                                   |  | |                            | | /usr/share/sec-lsm-manager/ | |                    | install               |
|                                   |  | |  app-redpesk-service-id2   | | selinux-policy/             | |                    |                       |
|                                   |  | |                            | |                             | |                    |                       |
|                                   |  | |                            | | redpesk-service-id.te       | |                    | uninstall             |
|                                   |  | |                            | |                             | |                    |                       |
|                                   |  | |                            | | redpesk-service-id.if       | |                    |                       |
|                                   |  | |                            | |                             | |                    | clear                 |
|                                   |  | |                            | | redpesk-service-id.fc       | |                    |                       |
|                                   |  | |                            | |                             | |                    |                       |
|                                   |  | |                            | | redpesk-service-id.pp       | |                    | display               |
|                                   |  | |                            | |                             | |                    |                       |
|                                   |  | +----------------------------+ +-----------------------------+ |                    |                       |
+-----------------------------------+  +----------------------------------------------------------------+                    +-----------------------+
```

## Compile

```bash
mkdir build
cd build
cmake ..
make
```

## Launch test

```bash
sudo make test
```
