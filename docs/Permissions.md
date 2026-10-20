# Permissions

In the sec-lsm-manager, the following system permissions are used : [redpesk-permissions]({% chapter_link application-framework.permissions %})

## Cynagora

Permissions are sent and stored in cynagora.
For example, if we add the permission `urn:redpesk:permission::partner:scope-platform`
to a demo-app we will have :

```
CLIENT    SESSION   USER   PERMISSION                                      RESULT  EXPIRE
demo-app  *         *      urn:redpesk:permission::partner:scope-platform  yes     forever
```

For more details on cynagora: [sec-cynagora](https://github.com/redpesk-core/sec-cynagora)

## Mandatory Access Control

At the moment permissions only have an impact on SELinux.
They allow adding authorizations to an application.
For example the `urn:redpesk:permission::partner:scope-platform` permission will allow access to the `/var/scope-platform` folder.

Here is a list of currently supported permissions and their effect :

- urn:redpesk:permission::partner:scope-platform
  > Allows access to the `/var/scope-platform` folder
- urn:redpesk:permission::partner:create-can-socket
  > Allows creation of a can socket and writing to it
- urn:redpesk:permission::partner:execute-shell
  > Allows execution of shell and programs in bin directories
- urn:redpesk:permission::partner:manage-tmp
  > Allows managing all files types in `/tmp`
- urn:redpesk:permission::partner:manage-user-shared
  > Allows managing all files types in `/run/user/[id]/usrshr/`
