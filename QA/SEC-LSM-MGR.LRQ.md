# Low level requirements of redpesk-core/sec-lsm-manager

.VERSION: DRAFT

.AUTHOR: José Bollo [IoT.bzh]

.AUDIENCE: ENGINEERING

.DIFFUSION: PUBLIC

The component redpesk-core/sec-lsm-manager is here denoted as
SEC-LSM-MANAGER

The document SEC-LSM-MGR.OVE describes SEC-LSM-MANAGER


## Content delivery

The delivery is made of 4 artifacts:

- the service SEC-LSM-MANAGER
- the client static library
- the administration tools

![Figure: Content delivery](assets/SEC-LSM-MGR.fig-delivery.svg)

### Content of the service delivery

The service delivery can exist in 3 different versions depending on what
LSM backend it handles.

#### Content of the full service delivery

.REQUIREMENT SEC-LSM-MGR.LRQ-R-CON-FUL-SER-DEL
The delivery of the service must contain the following items.

This delivery is made of the following artifacts:

- the binaries implementing the service
- the configuration files for the policy
- the service files for systemd integration

The binaries are:

- `/usr/bin/sec-lsm-managerd`: the switching service that detects the current
  LSM backend and runs the corresponding service for that backend
- `/usr/bin/sec-lsm-manager-smackd`: the service for Smack LSM backend
- `/usr/bin/sec-lsm-manager-selinuxd`:the service for SELinux LSM backend

The configuration files are:

- `/usr/share/sec-lsm-manager/app-template.smack`: the configuration file of
  the service for Smack LSM backend
- `/usr/share/sec-lsm-manager/app-template.if`: the interface configuration
  file of the service for SELinux LSM backend
- `/usr/share/sec-lsm-manager/app-template.te`: the type enforcement
  configuration file of the service for SELinux LSM backend

The service files for systemd are:

- `/lib/systemd/system/sec-lsm-manager.service`: the service description file
- `/lib/systemd/system/sec-lsm-manager.socket`: the socket activation file
- `/lib/systemd/system/sockets.target.wants/sec-lsm-manager.socket`: the link
  starting the activation mechanosm for the socket
- `/lib/systemd/system/sec-lsm-manager.target`: the target for managing
  dependencies to the service

#### Content of the Smack service delivery

.REQUIREMENT SEC-LSM-MGR.LRQ-R-CON-SMA-SER-DEL
The delivery of the service specialized for Smack LSM backend must contain
the itemzs of the full service except the items related to SELinux LSM
backend.

#### Content of the SELinux service delivery

.REQUIREMENT SEC-LSM-MGR.LRQ-R-CON-SEL-SER-DEL
The delivery of the service specialized for SELinux LSM backend must contain
the itemzs of the full service except the items related to Smack LSM
backend.

### Content of the client static library delivery

.REQUIREMENT SEC-LSM-MGR.LRQ-R-CON-CLI-STA-LIB-DEL
The client static library delivery must contain the following artifacts
needed for developping clients:

- `/usr/lib64/libsec-lsm-manager.a`: the static library
- `/usr/include/sec-lsm-manager.h`: the C header for content of library

### Content of the administration tools delivery

.REQUIREMENT SEC-LSM-MGR.LRQ-R-CON-ADM-TOO-DEL
The administration tools delivery must contain the following artifacts:

- `/usr/bin/sec-lsm-manager-cmd`: the client tool










## Software components

Internally, SEC-LSM-MANAGER is made of the below components:

![Figure: components of sec-lsm-manager](assets/SEC-LSM-MGR.fig-components.svg)

- **tool**: program for administration and test, based on using the library
- **library**: provides a client C API for handling server connection
- **protocol**: translates internal view of messages to and from bytestreams
- **server**: receives client queries and translates it to actions
- **permission**: checks permissions or manages it (creation, removal), linked
  with CYNAGORA
- **system**: applies policy rules to the system
- **template**: translates logical policy to system policy using templating
























## The Service

### Service

.REQUIREMENT SEC-LSM-MGR.LRQ-R-SER
SEC-LSM-MANAGER is a service. It must acts in response of other
programs requests (specifically the application manager).
When invoked in command line it must have one of the three
below behaviour:

- print a short help when invoked with argument `--help` and exit
- print its version when invoked with argument `--version` and exit
- remain running and wait for client connections until killed

**MOTIVATION**:
The SEC-LSM-MANAGER should not be invoked in command line but
should be integrated in the system configuration for running as service.

### UDS only

.REQUIREMENT SEC-LSM-MGR.LRQ-R-UDS-ONL
SEC-LSM-MANAGER must accept client connection only on a
protected UNIX domain socket.

**MOTIVATION**: Unix domain sockets are able to report the credentials of the
pair connected. This feature is important for SEC-LSM-MANAGER that need to
check the credential of its clients.

### Socket activation

.REQUIREMENT SEC-LSM-MGR.LRQ-R-SOC-ACT
The SEC-LSM-MANAGER service must start automatically using the UDS socket
activation mechanism. On systems using systemd, two systemd files must exist
to achieve that behaviour: `sec-lsm-manager.service` and
`sec-lsm-manager.socket`.

**MOTIVATION**: Most of the time, SEC-LSM-MANAGER is not needed because it
is needed only when installing or deinstalling applications. So it should
not be normally started. Nevertheless, it must be available when needed
without any explicit action.

### Socket access protection

.REQUIREMENT SEC-LSM-MGR.LRQ-R-SOC-ACC-PRO
Access to the UDS socket of the SEC-LSM-MANAGER must be possible only by
client having the identity *sec-lsm-manager* or the group *sec-lsm-manager*
or the capability *CAP_DAC_OVERRIDE*.

**MOTIVATION**:
Connection to SEC-LSM-MANAGER manger service are protected.
But even if such protection is reliable, the service SEC-LSM-MANAGER
is critical and must then check the credentials of its clients.




### User identity of the service

.REQUIREMENT SEC-LSM-MGR.LRQ-R-USE-IDE-SER
The SEC-LSM-MANAGER must run as the user *sec-lsm-manager* and
the group *sec-lsm-manager*.

**MOTIVATION**:
This protects the SEC-LSM-MANAGER from being killed by other programs
because it is the only program running as user *sec-lsm-manager*.

### Few capabilities only

.REQUIREMENT SEC-LSM-MGR.LRQ-R-FEW-CAP-ONL
The SEC-LSM-MANAGER must have only the following Linux capabilities:

- `CAP_DAC_OVERRIDE`
- `CAP_DAC_READ_SEARCH`
- `CAP_FOWNER`
- `CAP_MAC_ADMIN`
- `CAP_MAC_OVERRIDE`
- `CAP_SETFCAP`
- `CAP_SYS_ADMIN`

**MOTIVATION**:
The SEC-LSM-MANAGER must have these capabilities to achieve its
functionnalities.

## Credentials

### User and group

.REQUIREMENT SEC-LSM-MGR.LRQ-R-USE-GRO
The service SEC-LSM-MANAGER must runs with a user ID and a group ID
that are not root. That Ids must be configuration items as detailled at
`SEC-LSM-MGR.ADM-USRGRP`.

**MOTIVATION**: The service must run as a not root user/group. Letting it
configurable is smart for system architects that integrates it.

### Linux capabilities

.REQUIREMENT SEC-LSM-MGR.LRQ-R-LIN-CAP
The SEC-LSM-MANAGER must have only the Linux capabilities it
needs and drop all other Linux capabilities that it doesn't require.

**MOTIVATION**: For safety in case of bug or compromising.

### List of Linux capabilities

.REQUIREMENT SEC-LSM-MGR.LRQ-R-LIS-LIN-CAP
The Linux capabilities kept by the SEC-LSM-MANAGER must be
listed in its administration manual at reference `SEC-LSM-MGR.ADM-CAP`.
Motivation for keeping these capabilities must be documented in developer
manual at reference `SEC-LSM-MGR.DEV-CAP` and must be referenced in the
administration manual.

**MOTIVATION**: Administrators should be able to check capabilities of the
SEC-LSM-MANAGER. Developers must tell what capabilities are required for
assuming the requirements.




































## Networking

### Stand alone service

.REQUIREMENT SEC-LSM-MGR.LRQ-R-STA-ALO-SER
The SEC-LSM-MANAGER is a service that must run as a
single stand-alone process using no dynamically linked library.

**MOTIVATION**:
The process must be isolated. It is important for
giving it the required Linux capabilities that will allow it to
perform its service.

Conversely, it is expected that other processes have no Linux privilege
or Linux capabilities. Consequently the SEC-LSM-MANAGER can not be implemented
as a library.

### Stream or ordered packets

.REQUIREMENT SEC-LSM-MGR.LRQ-R-STR-ORD-PAC
The transport must be either stream oriented or ordered packet orineted.

**MOTIVATION**:
As discussed below (see @#presentation-layer), the socket type can be
either `SOCK\_STREAM` or `SOCK\_SEQPACKET`. For historical reason it is
currently set to `SOCK\_STREAM`. But because clients are required to acces
SEC-LSM-MANAGER using the library and because networking between remotes
are forbidden (see @SEC-LSM-MGR.HRQ-R-LOC-SER), it is possible to change that behaviour.


### No networking

.REQUIREMENT SEC-LSM-MGR.LRQ-R-NO-NET
The service SEC-LSM-MANAGER must not be linked to any
networking interface (except the required Unix Domain Socket IPC mechanism).

**MOTIVATION**: The service SEC-LSM-MANAGER is specific to the device and
should not be driven by external entity directly.

### Tuning of policies

.REQUIREMENT SEC-LSM-MGR.LRQ-R-TUN-POL
The process of translating logical policies to
real policies must be tunable through configuration file(s).

**MOTIVATION**:
While almost fixed, the translation should be
opened and be enough flexible to be adapted to newer or
specific systems. So changing the policy without changing the
code allows to only certificate the configuration of the policy.

**NOTE**:
It is an implementation option to embed the policy
in the binary of the server, making it really standalone.


































### The protocol

The protocol normalize the exchanges between the SEC-LSM-MANAGER,
the server, and its clients.

It is made of 3 components:

- the component **prot**: implementation of the presentation layer
- the component **sec-lsm-manager**: implementation of the application
  layer for the client side
- the component **sec-lsm-manager-server**: implementation
  of the application layer for the server side

#### Component prot

The component *prot* implement the presentation layer.

Its class decomposition is given on the **figure class prot**:

![Figure: class prot](assets/SEC-LSM-MGR.fig-class-prot.svg)

That component is used by the components *sec-lsm-manager* and
*sec-lsm-manager-server*.

The class `prot` is in the C files `prot.c` and `prot.h`.

It must fulfil the requirement @SEC-LSM-MGR.LRQ-R-PRE-LAY.

#### Component sec-lsm-manager

That component exposes a user interface for communicating with
service SEC-LSM-MANAGER.

This is a single class as shown on the **figure class sec-lsm-manager**

![Figure: class prot](assets/SEC-LSM-MGR.fig-class-sec-lsm-manager.svg)

That single class embbeds an instance of *prot* and uses *open* feature
of *socket* to open the socket given its designation.

#### Component sec-lsm-manager

That component holds client connection and the dispatching of decoded
imputs.

### The library

The library is static. It exports only the component `sec-lsm-manager`
and its related needs (socket and prot).


### The tool

The tool is binary program that can be used 4 of different ways:

- getting its version (argument -v or --version)
- getting a short help (argument -h or --help)
- submitting commands in script
- submitting commands interactively in a console

The main usage is for testing so it will be mostly used interactively.

The tool is made of components:

- the component **scanner**: implement scanning of command lines
- the component **interpreter**: implement interpretation of
  commands as actions

The actions are either help or interaction with the server.






















## Client library

### A library for clients

.REQUIREMENT SEC-LSM-MGR.LRQ-R-LIB-CLI
The project must deliver a static client library exposing
a high level C API conforming to the logical view of operations.

**MOTIVATION**: The protocol between the client and SEC-LSM-MANAGER is
probably public and could be coded by client. But, it is safer to
provide a tested and error prone client library for integration.
The advantage of static library is that it's more  difficult to tamper.

### Clients use the library

.REQUIREMENT SEC-LSM-MGR.LRQ-R-CLI-USE-LIB
The client should use the library for interacting with SEC-LSM-MANAGER.

**MOTIVATION**:
The protocol of SEC-LSM-MANAGER could evolve or its transport
could change. Relying on a C API is safer.

## The protocol

The protocol normalize the exchanges between the SEC-LSM-MANAGER,
the server, and its clients.

The protocol is built on a text line encoding of its components.
It is conceptualy splitted in two parts that follow OSI layering:
presentation and application.

### Presentation layer

The role of the presentation layer is to split the received
frames in lines and the lines in its fields. Conversely it
translates an orderred set of fields to a line frame.

The presentation layer accept stream transport and is compatible
with packet transport if lines are fully contained in a single
packet and if packets are delivered in the sent order (ex. SOCK\_SEQPACKET).

.REQUIREMENT SEC-LSM-MGR.LRQ-R-PRE-LAY
The presentation layer is oriented line of text and must conform
to the grammar shown on below figure.

![Figure: presentation layer](assets/SEC-LSM-MGR.fig-protocol-presentation.svg)

**MOTIVATION**:
Text oriented protocols have the advantage to be readable.
It is better for implementing diagnostic tools or process.

**NOTE**:
Empty fields are authorized by the presentation layer.

### Application layer

The role of the application layer is to fix the possible exchanges.
It tells what can be send when by what part (client or server) and

That means that internal instanciation of that layer is not enforced
to have a data representation counterpart.


![Figure: state protocol](assets/SEC-LSM-MGR.fig-protocol-grammar.svg)


![Figure: state protocol](assets/SEC-LSM-MGR.fig-protocol-states.svg)

![Figure: state protocol](assets/SEC-LSM-MGR.fig-install-states.svg)

#### Protocol violation

.REQUIREMENT SEC-LSM-MGR.LRQ-R-PRO-VIO
When the SEC-LSM-MANAGER detects a violation of protocol on a client
connection, it must close the connection without explaination and drop
any context associated with the faulty client.





























