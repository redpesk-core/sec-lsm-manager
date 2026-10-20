# Compilation

## Dependencies

Deploy a `SDK` container: [Overview]({% chapter_link sdk-container-doc.overview %})

Smack is not packaged and has to be compiled from source.

```bash
sudo dnf install check-devel sec-cynagora-devel libsemanage-devel libselinux-devel systemd-devel libcap-devel
```

Build Smack:

```bash
sudo dnf install autoconf automake libtool
git clone https://github.com/smack-team/smack
cd smack
./autogen.sh
make
sudo make install
```
## Options

It is possible to modify the following compilation options when executing the cmake command:

- **WITH_SYSTEMD** (default: `ON`): systemd socket activation
- **WITH_SMACK** (default: `OFF`) : SMACK mode
- **WITH_SELINUX** (default: `OFF`): SELinux mode

+ **WITH_SIMULATION** (default: `OFF`): activates simulations for cynagora, SMACK and SELinux
+ **SIMULATE_CYNAGORA** (default: `OFF`): simulates cynagora
+ **SIMULATE_SMACK** (default: `OFF`): simulates SMACK
+ **SIMULATE_SELINUX** (default: `OFF`): simulates SELinux

- **FORTIFY** (default: `ON`): fortifies source code
- **COMPILE_TEST** (default: `ON`): compiles tests
- **DEBUG** (default: `OFF`): activates debug mode (symbols, debug messages)

For example with DEBUG option and only SELinux:

```bash
cmake -DDEBUG=ON -DWITH_SELINUX=ON ..
```

## Environment Variables

There are variables defined at compile time, in the `CMakeLists.txt` file, that can be changed at runtime by setting an environment variable.

For example the **SEC_LSM_MANAGER_SOCKET_NAME** variable which contains `sec-lsm-manager.socket` can be modified at launch:

```bash
export SEC_LSM_MANAGER_SOCKET_NAME="new-socket-name.socket"
/usr/bin/sec-lsm-managerd
```

It is possible to modify the following environment variables:

- **SELINUX_RULES_DIR** (default: `/usr/share/sec-lsm-manager/selinux-rules`)
- **SELINUX_MAKEFILE** (default: `/usr/share/selinux/devel/Makefile`)
- **SEC_LSM_MANAGER_DATADIR** (default: `/usr/share/sec-lsm-manager`)
- **SEC_LSM_MANAGER_SOCKET_NAME** (default: `sec-lsm-manager.socket`)

+ **COMPILE_SCRIPT_DIR** (default: `/usr/share/sec-lsm-manager/script`)
+ **COMPILE_SCRIPT_NAME** (default: `build-module.sh`)

- **TE_TEMPLATE_FILE** (default: `app-template.te`)
- **IF_TEMPLATE_FILE** (default: `app-template.if`)
- **TEMPLATE_FILE** (default: `app-template.smack`)

+ **SELINUX_FS_PATH** (default: `/sys/fs/selinux`)
+ **SMACK_FS_PATH** (default: `/sys/fs/smackfs`)

- **SMACK_POLICY_DIR** (default: `/etc/smack/accesses.d`, simulation: `/usr/share/sec-lsm-manager/smack-simulation`)
- **SELINUX_POLICY_DIR** (simulation: `/usr/share/sec-lsm-manager/selinux-simulation`)
