# Software design document of redpesk-core/sec-lsm-manager

.VERSION: DRAFT
.AUTHOR: José Bollo [IoT.bzh]

The component redpesk-core/sec-lsm-manager is here denoted as
SEC-LSM-MANAGER

## Components

Internally, SEC-LSM-MANAGER is made of the below components:

![Figure: components](assets/SEC-LSM-MGR.fig-components.svg)

- **tool**: program for administration and test, based on using the library
- **protocol**: translation of messages to and from bytestreams
- **client**: provides a library client C API for connecting to server
- **server**: receives client queries and translates it to actions
- **context**: holds a client context
- **action**: performs install and uninstall
- **MAC-LSM**: applies policy rules to the system
- **permission**: checks permissions or manages it, linked with cynagora
- **templating**: translates logical policy to system policy using templating

## The protocol

The protocol normalize the exchanges between the SEC-LSM-MANAGER,
the server, and its clients.

It is made of 3 components:

- the component **prot**: implementation of the presentation layer
- the component **sec-lsm-manager-client**: implementation of the application
  layer for the client side
- the component **sec-lsm-manager-server**: implementation
  of the application layer for the server side

### Component prot

The component *prot* implement the presentation layer.
The presentation layer is fully described in @SEC-LSM-MGR.PRO

Its class decomposition is given on the **figure class prot**:

![Figure: class prot](assets/SEC-LSM-MGR.fig-class-prot.svg)

The class `prot` is used by the components *sec-lsm-manager-client* and
*sec-lsm-manager-server*. Both components use it for sending and
receiving records of the protocol.

#### Sizes and constants

The class prot defines the encoding characters as being:

- SPACE for field separator `PROT_FIELD_SEPARATOR`
- LINE-FEED for record separator `PROT_RECORD_SEPARATOR`
- ESCAPE for escaper `PROT_ESCAPE`

It also defines the **maximum sizes**:

- `PROT_MAX_BUFFER_LENGTH` the maximum received or emitted size
  in bytes for a record
- `PROT_MAX_FIELDS` the maximum count of fields in reception

#### Live cycle and setting

The functions create and destroy allocate or deallocate a prot
instance in memory.

The function reset reset the `prot` instance to its default value.

The class prot allows only one setting: allowing or not the records
without any field. That setting is controlled by functions `is_empty_allowed`
and `set_allow_empty`. The effect is:

- if empty records are not allowed, no empty record is never writen
  and if an empty record is read, it is not transmitted to the application
- if empty records are allowed, empty records are written and received
  empty records are delivered to the application.

#### Encoding and writing

Sending operation is made of two parts: encoding and writing.

Encoding a record is made using the basic function `put_field`
that add one field to the record, encoding it in a buffer.

The end of the record is encoded using the function `put_end`.
Once the end is put, the record is ready to be transmitted using
the function `write`.

For facitily, the following functions are also availables

- `put_fields`: add a set of fields to the record
- `put`: add a set of fields to the record and ends the record
- `putx`: add a variable count of fields to the record and ends the record
- `put_cancel`: abort current record composition

#### Reading and decoding

The function `can_read` tels if reading is possible. The function `read`
can then be called for filling the read buffer.

Then extracting the fields from the read records is achieved using the
function `get`. The returned fields are available, pointing directly
to the memory held by the receive buffer until the next record is
required by calling the function `next`.

#### Internal objects

The class prot **internally** uses two other classes: fields and buf.

![Figure: class prot](assets/SEC-LSM-MGR.fig-class-prot-intern.svg)

The internal class buf essentially manages a ring buffer used by
the class prot for:

- buffering the decoded data received and presenting it
- buffering encoded data before transmission

Thus the correct integration involves two instances of the class
buf: inbuf for reception and outbuf for transmission.

On transmission, the buffer outbuf is used as a pure ring buffer
and the posix function writev is used to atomicaly transmit the
data in a single operation.

On reception, the buffer shifted in order to ensure that the presentation
of the received data to the application is not wrapping around the buffer.

The class fields is used when decoding for recording position
of fields in the decoding buffer.


### Component sec-lsm-manager-client

That component exposes a user interface for communicating with
service SEC-LSM-MANAGER.

This is a single class as shown on the **figure class sec-lsm-manager**

![Figure: class sec_lsm_manager_client](assets/SEC-LSM-MGR.fig-class-sec-lsm-manager-client.svg)

That single class embbeds an instance of *prot* and uses *open* feature
of *socket* to open the socket given its designation.

#### Life cycle and meta

Instances of `sec_lsm_manager_client` are created using function `create` that
connects the instance to the specified socket designated by a name. The
function `destroy` deletes it and clean the memory. The function `disconnect`
disconnects from the server but doesn't destroys the instance.

The function `log`allows tuning the logging of the server (see log of
@SEC-LSM-MGR.PRO).

The function `error_message` returns a copy of the error indication of
the latest record received from the server.

#### Context and action

The other functions are directly linked to the management of the context
by the server, each of them sends a record corresponding to required
intention as described by the protocol.


### Component sec-lsm-manager-server

That component holds client connection and the dispatching of decoded
imputs.

The figure below shows its organization:

![Figure: class sec_lsm_manager_server](assets/SEC-LSM-MGR.fig-class-sec-lsm-manager-server.svg)

The class `sec_lsm_manager_server` has a listening socket for accepting
incoming clients. The incoming clients are then attached to an instance
of the private class `client`.

The instance of `sec_lsm_manager_server` and instances of its `client`
are inheriting the class `pollitem` for inserting it in an event loop
based on the system call `epoll_wait`. It means that 

Each instance of the class `client` is composed with one instance
of the class `prot` and one instance of the class `context`.

The class server interprets for each of its client the received
messages.


## The context


![Figure: class context](assets/SEC-LSM-MGR.fig-class-context.svg)


![Figure: classes used for the context](assets/SEC-LSM-MGR.fig-class-context-items.svg)


## The action



![Figure: class action](assets/SEC-LSM-MGR.fig-class-action.svg)




## The tool

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






## Content delivery

The delivery is made of 3 artifacts:

- the service SEC-LSM-MANAGER
- the client static library
- the client tools for administration and test

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

The library offers to client of SEC-LSM-MANAGER the recommended way to interact
with the SEC-LSM-MANAGER. The library encapsulate details of the protocol and
present to it clients the clean inteface of the class `sec-lsm-manager-client`.

The library is provided in its static version in order to be linked statically
to its clients. The reason of doing so is for removing an attackabke item.



### Content delivery of the client tool

- the client tool binary
- the manual page for using it



