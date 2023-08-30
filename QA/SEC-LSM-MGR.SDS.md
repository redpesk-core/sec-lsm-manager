# Software design document of redpesk-core/sec-lsm-manager

.VERSION: DRAFT
.AUTHOR: José Bollo [IoT.bzh]

The component redpesk-core/sec-lsm-manager is here denoted as
SEC-LSM-MANAGER

## Content delivery

The delivery is made of 3 artifacts:

- the service SEC-LSM-MANAGER
- the client static library
- the client tools for interacting manually with the service

![Figure: Content delivery](assets/SEC-LSM-MGR.fig-delivery.svg)

### Content delivery of the service

This delivery is made of the following artifacts:

- the binary implementing the service
- the configuration files for the policy
- the configuration files for systemd integration
- the administration guide

### Content delivery of the client static library

- the static client library
- one C header file declaring and summarizing
  the exported symbols of the library
- the integration guide for developpers

### Content delivery of the client tool

- the client tool binary
- the manual page for using it


## Components

Internally, SEC-LSM-MANAGER is made of the below components:

![Figure: components](assets/SEC-LSM-MGR.fig-components.svg)

- tool: program for administration and test, based on using the library
- library: provides a client C API for handling server connection
- protocol: translates internal view of messages to and from bytestreams
- server: receives client queries and translates it to actions
- permission: checks permissions or manages it (creation, removal), linked with cynagora
- system: applies policy rules to the system
- template: translates logical policy to system policy using templating

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



...



