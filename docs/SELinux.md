# SELinux

## Policy Module

A policy package will define the rules of your application and it is easy to install and uninstall.

SElinux Modules are stored in the directory :

```
/usr/share/selinux/default
```

Three files in a policy package :

- Type Enforcement (.te)

- File Context (.fc)

- Interfaces (.if)


### Type Enforcement files

Type enforcement files contains the rules to confine your application


### File Context files

File contexts files define which label to give to each files


### Interface files

Interface files contain interfaces to says how they interact with the modules

## Development Environment

### Create

For this example, we work with a fedora 31.

To create rule easier you need to have a good environment.

First, create these repositories with an unconfined_u or sysadm_u :

```
~/dev/
~/dev/selinux/
~/dev/selinux/bin
~/dev/selinux/centralized
~/dev/selinux/local
```

In the ~/dev/selinux/bin copy the file functions.sh

```
https://resources.oreilly.com/examples/9781783989669/blob/master/functions.sh
```

This file was developed by Sven Vermeulen and makes it easier to read the current policies.

This file need the environment variable define : ${POLICY_LOCATION}

Add the following line to your .bashrc:

```
DEVROOT=~/dev/selinux
POLICY_LOCATION=${DEVROOT}/centralized/selinux-policy
source ${DEVROOT}/bin/functions.sh
```

Now let's clone the current policy of fedora 31 in the repository ~/dev/selinux/centralized :

```bash
git clone https://github.com/fedora-selinux/selinux-policy
git submodule update --init --recursive
git checkout f31
```

Finally, create a symbolic link with the Makefile, that will allow to build module in the repository ~/dev/selinux/local:

```bash
ln -s /usr/share/selinux/devel/Makefile ~/dev/selinux/local/
```

### Use

Now that the environment is correctly created, you can create new modules (te, fc, if file) in the local repository.

You have access to the functions :

- sefindif : Allow to find an interface
- seshowif : Allow to show an interface
- sefinddef : Allow to find a definition
- seshowdef : Allow to show a definition

## Examples

### Application

We will create an application that will open a server and communicate on port 1234, all messages that it receive will be stored temporarily.

The name of the application is **server** and it is stored in /opt/server/server.py

```python
#!/usr/bin/python3

__author__ = "Arthur Guyader"
__copyright__ = "Copyright (C) 2020-2021 'IoT.bzh Company'"
__license__ = "Apache 2.0"
__version__ = "1.0"
__maintainer__ = "Arthur Guyader"
__email__ = "arthur.guyader@iot.bzh"

import socket
import sys

def create_server():
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.bind(('',1234))
        print("Start server on localhost:1234")
        s.listen(5)
        return s

def wait_connection(s):
        while True:
                print("Wait connection...")
                return s.accept()

def recv_msg(connection, address):
        data = connection.recv(1024)
        print("Receive data from {} = {}".format(address, data))
        if data:
                save_data(data)

def save_data(data):
        file = open("/tmp/server_data.txt","ab")
        file.write(data)
        file.close()

def close_server(connection, s):
        print("Close server")
        if connection:
                connection.close()
        if s:
                s.close()

def main():
        try:
                s = create_server()
                (connection, address) = wait_connection(s)
                print("Connection from : {}".format(address))
                recv_msg(connection,address)
        finally:
                close_server(connection,s)

main()
```

### SELinux Rules

Let's create our modules for SELinux:

#### myserver.te

So first we need to create a type enforcement file :

```
###########################################################################
# Copyright 2020-2021 IoT.bzh Company
#
# Author: Arthur Guyader <arthur.guyader@iot.bzh>
#
# $RP_BEGIN_LICENSE$
# Commercial License Usage
#  Licensees holding valid commercial IoT.bzh licenses may use this file in
#  accordance with the commercial license agreement provided with the
#  Software or, alternatively, in accordance with the terms contained in
#  a written agreement between you and The IoT.bzh Company. For licensing terms
#  and conditions see https://www.iot.bzh/terms-conditions. For further
#  information use the contact form at https://www.iot.bzh/contact.
#
# GNU General Public License Usage
#  Alternatively, this file may be used under the terms of the GNU General
#  Public license version 3. This license is as published by the Free Software
#  Foundation and appearing in the file LICENSE.GPLv3 included in the packaging
#  of this file. Please review the following information to ensure the GNU
#  General Public License requirements will be met
#  https://www.gnu.org/licenses/gpl-3.0.html.
# $RP_END_LICENSE$
###########################################################################


policy_module(myserver,1.0)
############################

# ------ DECLARATIONS ------ #

#### TYPES ####
type server_t;
type server_exec_t;
type server_log_t;
type server_tmp_t;

#### REQUIRE ####

gen_require(`
    type monopd_port_t;
    type node_t;
')

#### DOMAIN ####
domain_type(server_t)
domain_entry_file(server_t, server_exec_t)

#### LOG ####
logging_log_file(server_log_t)

#### TMP ####
files_tmp_file(server_tmp_t)

# ------ POLICY ------ #

allow server_t server_log_t:file append_file_perms;
allow server_t server_tmp_t:file manage_file_perms;

# Allow create socket

allow server_t server_t:tcp_socket create_stream_socket_perms;

# Allow bind on port 1234
allow server_t monopd_port_t:tcp_socket name_bind;
allow server_t node_t:tcp_socket node_bind;

# Allows server_t to create file in tmp we label server_tmp_t
files_tmp_filetrans(server_t,server_tmp_t,file)

# Allow access tty and devpts
userdom_use_user_terminals(server_t)

# Allow map on bin_t
corecmd_mmap_bin_files(server_t)
```

List of permissions : https://selinuxproject.org/page/NB_ObjectClassesPermissions

In this file we define the permission of our application. So we create four new types :

- server_t
- server_exec_t
- server_log_t
- server_tmp_t

Our application will be labeled with the type server_exec_t and when we will execute it, it will take the type server_t.

The logs of the application will be stored in a file with server_log_t type.

And our temporary data will be stored with the type server_tmp_t.

If you want to have more information about interfaces you can for example execute the command :

```
seshowif logging_log_file
```

#### myserver.fc

We also need to have the good label on our file /opt/server/server.py :

```
###########################################################################
# Copyright 2020-2021 IoT.bzh Company
#
# Author: Arthur Guyader <arthur.guyader@iot.bzh>
#
# $RP_BEGIN_LICENSE$
# Commercial License Usage
#  Licensees holding valid commercial IoT.bzh licenses may use this file in
#  accordance with the commercial license agreement provided with the
#  Software or, alternatively, in accordance with the terms contained in
#  a written agreement between you and The IoT.bzh Company. For licensing terms
#  and conditions see https://www.iot.bzh/terms-conditions. For further
#  information use the contact form at https://www.iot.bzh/contact.
#
# GNU General Public License Usage
#  Alternatively, this file may be used under the terms of the GNU General
#  Public license version 3. This license is as published by the Free Software
#  Foundation and appearing in the file LICENSE.GPLv3 included in the packaging
#  of this file. Please review the following information to ensure the GNU
#  General Public License requirements will be met
#  https://www.gnu.org/licenses/gpl-3.0.html.
# $RP_END_LICENSE$
###########################################################################


# server.py will have :
# label : system_u:object_r:server_exec_t
/opt/server/server.py  -- gen_context(system_u:object_r:server_exec_t,s0)
```



If you create the rule after the creation of the file, you need to relabel :

```bash
restorecon -F /opt/server/server.py
```

#### myserver.if

Finally, we need to define an interface to say how we will interact with our application:

```
###########################################################################
# Copyright 2020-2021 IoT.bzh Company
#
# Author: Arthur Guyader <arthur.guyader@iot.bzh>
#
# $RP_BEGIN_LICENSE$
# Commercial License Usage
#  Licensees holding valid commercial IoT.bzh licenses may use this file in
#  accordance with the commercial license agreement provided with the
#  Software or, alternatively, in accordance with the terms contained in
#  a written agreement between you and The IoT.bzh Company. For licensing terms
#  and conditions see https://www.iot.bzh/terms-conditions. For further
#  information use the contact form at https://www.iot.bzh/contact.
#
# GNU General Public License Usage
#  Alternatively, this file may be used under the terms of the GNU General
#  Public license version 3. This license is as published by the Free Software
#  Foundation and appearing in the file LICENSE.GPLv3 included in the packaging
#  of this file. Please review the following information to ensure the GNU
#  General Public License requirements will be met
#  https://www.gnu.org/licenses/gpl-3.0.html.
# $RP_END_LICENSE$
###########################################################################

interface(`server_domtrans',`
        gen_require(`
                type server_t, server_exec_t;
        ')
        domtrans_pattern($1,server_exec_t,server_t)
')
```

Let's go in the directory ~/dev/selinux/local and compile your rule :

```bash
make
```

And install it:

```bash
semodule -i myserver.pp
```



### Usage

Now that we have correctly installed our policy we want to use it.

We will allow a user with the context of staff_u to execute the application.

So let's create a type enforcement to do that :

#### mystaff.te

```
###########################################################################
# Copyright 2020-2021 IoT.bzh Company
#
# Author: Arthur Guyader <arthur.guyader@iot.bzh>
#
# $RP_BEGIN_LICENSE$
# Commercial License Usage
#  Licensees holding valid commercial IoT.bzh licenses may use this file in
#  accordance with the commercial license agreement provided with the
#  Software or, alternatively, in accordance with the terms contained in
#  a written agreement between you and The IoT.bzh Company. For licensing terms
#  and conditions see https://www.iot.bzh/terms-conditions. For further
#  information use the contact form at https://www.iot.bzh/contact.
#
# GNU General Public License Usage
#  Alternatively, this file may be used under the terms of the GNU General
#  Public license version 3. This license is as published by the Free Software
#  Foundation and appearing in the file LICENSE.GPLv3 included in the packaging
#  of this file. Please review the following information to ensure the GNU
#  General Public License requirements will be met
#  https://www.gnu.org/licenses/gpl-3.0.html.
# $RP_END_LICENSE$
###########################################################################

policy_module(mystaff, 0.1)

gen_require(`
        type staff_t;
        role staff_r;
        type server_t;
')

# Allow domain transition for staff_t to server_t
server_domtrans(staff_t)

# Allow staff_r role to have the type server_t
role staff_r types server_t;
```

We also compile these rules and install it :

```bash
make
semodule -i mystaff.pp
```

Let's execute the application :

```bash
$ /opt/server/server.py
```

❗WARNING

If you launch the application like this :

```
python3 /opt/server/server.py
```

That will not work because you will execute the python3 binary and you don't define transition for it.

## Sources

Vermeulen, S. (2015). *Selinux Cookbook*. Packt Publishing.

https://mgrepl.fedorapeople.org/PolicyCourse/writingSELinuxpolicy_MUNI.pdf
https://debian-handbook.info/browse/fr-FR/stable/sect.selinux.html

https://selinuxproject.org/page/RefpolicyWriteModule
