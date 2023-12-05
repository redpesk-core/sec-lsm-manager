# Overview of redpesk-core/sec-lsm-manager

.VERSION: 2.6.1

.REVIEW: IREV1

## Overview

The component SEC-LSM-MANAGER must be the only component allowed to
administrate Linux Security Modules (LSM) and the permissions database
CYNAGORA.

Administration is needed at least for adding and removing managed
applications. For these actions, the system must:

- update the security policy of LSM and CYNAGORA
- update the file system accordingly to new LSM policy

None of these actions is possible without the necessary privileges.
The SEC-LSM-MANAGER has the required privileges.

LSMs are tightly bound to the kernel and can't be removed without access
to the kernel arguments at boot. The 2 targeted LSMs are:

- SELinux, the widely used mandatory access control for Linux
- SMACK, a lightweight mandatory access control for Linux
  well suited for embedded systems

The permission database (CYNAGORA) is a service running outside of
the kernel. Its administrative access is protected.

The figure shows the component SEC-LSM-MANAGER in the system:

![Figure: the component SEC-LSM-MANAGER](assets/SEC-LSM-MGR.OVE.fig-1.svg)

Because of its position in the system, the setup interface
that SEC-LSM-MANAGER offers must be protected.

The interface offered by SEC-LSM-MANAGER must hide the details
of how security is implemented in the real system. Instead, it
offers a logical view of operations, as shown on below figure.

![Figure: the component SEC-LSM-MANAGER](assets/SEC-LSM-MGR.OVE.fig-3.svg)

The interface of SEC-LSM-MANAGER establishes a separation between
the logical policy of the installed application and the effective
real policy as seen by the kernel. This feature allows the
SEC-LSM-MANAGER to deal with either SELinux or with SMACK.

## Main use case

The SEC-LSM-MANAGER allows an application manager to install and
to remove an application.

![Figure: use of SEC-LSM-MANAGER](assets/SEC-LSM-MGR.OVE.fig-2.svg)

When installing files, the application manager checks the package to be
installed. When it is validated, it asks the security manager
SEC-LSM-MANAGER to setup the security policy for the installed
application.

Uninstallation follows the same path.

## Discussion

The SEC-LSM-MANAGER is bound with the application manager
that is its only known client.

The reason of splitting the application manager in these two components
are:

- historical: that is the way it was done in Tizen
- separation of concerns and of privileges: the application
  manager does not need any privilege
- double check: the SEC-LSM-MANAGER has its own separate checks
- the application manager does not depend on the kernel LSM used
- emphasis on the separation between logical policy and real policy
