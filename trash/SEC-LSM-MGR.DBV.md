# Detailled behaviour of redpesk-core/sec-lsm-manager

.VERSION: DRAFT
.AUTHOR: José Bollo [IoT.bzh]

The component redpesk-core/sec-lsm-manager is here denoted as
SEC-LSM-MANAGER

The document SEC-LSM-MGR.OVE describes SEC-LSM-MANAGER

## Setting policy of a file

The client specify the policy to be applied to a file
by submitting the path of the file and the logical type
of that file.

Files are generally linked to an application The logical type of files

The logical types are: default, conf, data, exec, http,
icon, id, lib, plug, public.

    type     description
    -------  --------------------------
    default  system wide default access (ro)
    conf     configuration file of an app
    data     data file of an app
    exec     executable file of an app
    http     served file of an app
    icon     icon file of an app
    id       root directory of an app
    lib      library file of an app
    plug     exported directory of an app
    public   public file of an app

All this type (except the type default) are specific
to an application and thus are needing the application id
to be set.


## Adding a permission to an application

The client specify a permission to be given to the application.
It is the role of the application framework to check if the permissions
can be granted or not.


## Pluging items to an other application

The client specify that the application exports a directory to
an other application. This behaviour is needed for delivery
of add-ons components.

