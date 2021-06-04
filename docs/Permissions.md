## Permissions

In the sec-lsm-manager, the following system permission is used : [redpesk-permissions](https://docs.redpesk.bzh/docs/en/master/developer-guides/3-permissions.html#the-permissions)

### Cynagora

Permissions are sent and stored in cynagora.
For example, if we add the permission `urn:AGL:permission::partner:scope-platform`
to a demo-app we will have :

```
CLIENT      SESSION,    USER,   PERMISSION                                      RESULT      EXPIRE
demo-app    *           *       urn:AGL:permission::partner:scope-platform      yes         forever
```

For more details on cynagora: [sec-cynagora](https://github.com/redpesk-core/sec-cynagora)


### Mandatory Access Control

At this time permissions only have an impact on SELinux.
They will allow to add additional authorizations to an application.

For example the `urn:AGL:permission::partner:scope-platform` permission
will allow access to the `/var/scope-platform` folder.

Here is a list of currently supported permissions and their effect :


- urn:AGL:permission::partner:scope-platform

> Allow access to the `/var/scope-platform` folder

- urn:AGL:permission::partner:create-can-socket

> Allow create and write on can socket

- urn:AGL:permission::partner:read-afbtest

> Allow read binding afbtest

- urn:AGL:permission::partner:execute-shell

> Allow execute shell and programs in bin directories

- urn:AGL:permission::partner:access-tmp

> Allow manage all files types in /tmp
