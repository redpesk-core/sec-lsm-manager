# Usage

Within an application, we can qualify our files according to several types:

- default: no specific type, use default
- conf:    config files
- data:    data files
- exec:    executable files
- http:    http files
- icon:    icon file
- id:      basename app directory
- lib:     libraries files
- plug:    plugin files
- public:  public files

Moreover an application can have particular rights, for example for a CAN socket creation.

The first thing to do is to send all this information to the sec-lsm-manager
so it can proceed with the installation.

## Library

To start using the library, we will create an handler for information:

```c
#include <sec-lsm-manager.h>
sec_lsm_manager_t *sec_lsm_manager = NULL;
sec_lsm_manager_create(&sec_lsm_manager);
```

### Install

We need to define an id to identify our application:

```c
sec_lsm_manager_set_id(sec_lsm_manager, "demo-app");
```

> An id can only be composed of alpha numeric character, '-' and '_'. It must also be composed of at least two characters.

We will then qualify the different files of our application:

```c
sec_lsm_manager_add_path(sec_lsm_manager, "/opt/demo-app/", type_id);

sec_lsm_manager_add_path(sec_lsm_manager, "/opt/demo-app/bin/", type_exec);
sec_lsm_manager_add_path(sec_lsm_manager, "/opt/demo-app/bin/launcher.sh", type_exec);

sec_lsm_manager_add_path(sec_lsm_manager, "/opt/demo-app/data/", type_data);
sec_lsm_manager_add_path(sec_lsm_manager, "/opt/demo-app/data/db.sqlite", type_data);
sec_lsm_manager_add_path(sec_lsm_manager, "/opt/demo-app/data/info.json", type_data);

sec_lsm_manager_add_path(sec_lsm_manager, "/opt/demo-app/etc/", type_conf);
sec_lsm_manager_add_path(sec_lsm_manager, "/opt/demo-app/etc/file1.conf", type_conf);
sec_lsm_manager_add_path(sec_lsm_manager, "/opt/demo-app/etc/file2.config", type_conf);

sec_lsm_manager_add_path(sec_lsm_manager, "/opt/demo-app/www/", type_http);
sec_lsm_manager_add_path(sec_lsm_manager, "/opt/demo-app/www/index.html", type_http);
sec_lsm_manager_add_path(sec_lsm_manager, "/opt/demo-app/www/style.css", type_http);
```

> A path must be composed of at least two characters.

You can then add permissions:

```c
sec_lsm_manager_add_permission(sec_lsm_manager, "urn:AGL::partner:create-can-socket");
```

> A permission must be composed of at least two characters.

For more information about permissions: [Permissions]({% chapter_link sec-lsm-manager.permissions-definition %})

And finally we can install our application security context:

```c
sec_lsm_manager_install(sec_lsm_manager);
```

### Uninstall

To uninstall the application security context, you must define its id:

```c
sec_lsm_manager_set_id(sec_lsm_manager, "demo-app");
sec_lsm_manager_uninstall(sec_lsm_manager);
```

### Additional

It is possible to display the status of a handler with the display function:

```c
sec_lsm_manager_display(sec_lsm_manager);
```

⚠️ If an error occurs, a flag is raised and it is impossible to continue without using the clear function

```c
sec_lsm_manager_clear(sec_lsm_manager);
```

It is also necessary to free the handle created at the end:

```c
sec_lsm_manager_destroy(sec_lsm_manager);
```

## Command Line

It is possible to use the previous functions easily on the command line
thanks to the binary: `sec-lsm-manager-cmd`.

```bash
$ sec-lsm-manager-cmd
>> initialization success
id demo-app
>> ok
path /opt/demo-app id
>> ok
permission urn:AGL::partner:create-can-socket
>> ok
display
################## SECURE APP ##################
id demo-app
path /opt/demo-app id
permission "urn:AGL::partner:create-can-socket"
################################################
>> ok
install
>> ok
```
