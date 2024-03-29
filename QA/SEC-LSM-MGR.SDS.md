# Software design document of redpesk-core/sec-lsm-manager

.VERSION: 2.6.1
.AUTHOR: José Bollo [IoT.bzh]

The component redpesk-core/sec-lsm-manager is here denoted as
SEC-LSM-MANAGER

## Components

Internally, SEC-LSM-MANAGER is made of the below components:

![Figure: components](assets/SEC-LSM-MGR.fig-components.svg)

- **protocol**: translation of messages to and from bytestreams
- **context**: holds a client context
- **action**: performs install and uninstall
- **permission**: checks permissions or manages it, linked with cynagora
- **MAC-LSM**: applies policy rules to the system
- **templating**: translates logical policy to system policy using templating

## The protocol

The protocol normalize the exchanges between the SEC-LSM-MANAGER,
the server, and its clients.

It is made of 2 components:

- the component **prot**: implementation of the presentation layer
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


### Component sec-lsm-manager-server

That component holds client connections and the dispatching of pending
inputs. Nominally the server only serves one client. However it
can serve more than one client but one by one under the same main thread.

The figure below shows its organization:

![Figure: class sec_lsm_manager_server](assets/SEC-LSM-MGR.fig-class-sec-lsm-manager-server.svg)

The class `sec_lsm_manager_server` has a listening socket for accepting
incoming clients. The incoming clients are then attached to an fresh instance
of the private class `client`.

The instance of `sec_lsm_manager_server` inherits the class `pollitem`
for managing an event loop based on the system call `epoll_wait`.

The events occuring on inputs of the server are:

- a new pair connects: the server creates a fresh instance of client
  to handle that new connection
- a pair sent data: the server asks the client instance to process
  its input
- a pair diconnects: the server destroys the client instance

Each instance of the class `client` is composed with one instance
of the class `prot` that manges the presentation of the protocol
and one instance of the class `context` that manages the current
context of the client.

The class server interprets for each of its client the received
messages.


## The context

The class context records the current context state of a client
connection. It is the recording of all the correctly interpreted
messages of the protocol.

![Figure: class context](assets/SEC-LSM-MGR.fig-class-context.svg)

The method `raise_error` sets the `error_flag` until a call to the
method `clear`. The error state is queried using the method `has_error`.

The methods `set_id`, `add_permission`, `add_path`, `add_plug` and `clear`
are the function implementing the corresponding protocol queries:
`id`, `permission`, `path`, `plug` and `clear`. It returns a numerical
status whose values are:

- `0`: succes
- `-EINVAL`: invalid parameter
- `-EEXIST`: already set or added
- `-ENOTRECOVERABLE`: context in error, requires a clear
- `-ENOMEM`: out of memory
- `-ENOENT`: path doesn't exist
- `-EACCES`: inaccessible path
- `-ENOTDIR`: not a directory

The function `has_permission` checks the list of permissions in
the current context and returns true if the permission is given.

The function `visit` is used to inspect the content of the
context.

Internally, the class context uses instances of `permission_set`, `path_set`
and `plugset` to handle its data.

![Figure: classes used for the context](assets/SEC-LSM-MGR.fig-class-context-items.svg)

These classes have little behaviour: initialisation, addition of item an
clearing of data


## The action

The class `action` performs installation or uninstallation of an application
described by a context, the current context for clients of SEC-LSM-MANAGER.

Installation and unstallation involves 2 parts: cynagora the permission
manager and the Mandatory Access Control selected Smack or SELinux.

![Figure: class action](assets/SEC-LSM-MGR.fig-class-action.svg)

The class `action` only has static method because it doesn't need
variable: all it needs is in the context.

It use the class `cynagora_interface` for:

- query the permission database when plug is required
- add permission on installation
- remove permissions on uninstallation

It uses the class `MAC` for:

- getting the label of the application for cynagora
- installing the files and rules
- uninstalling the files and rules

The implementation of the effective class MAC and its
*virtual* static classes is made using aliasing
mechanism.

On its side, action performs two checks before processing:

- `check_context`: checks the context and retrieves
  the application label as expected by the LSM
- `check_plug_installable`: check that plug requests
  are valid and permitted (that check implies cynagora
  queries)

## The Smack action

Smack action is called for 3 different purposes:

- `get_label`: Getting the Smack application label for populating Cynagora
  permission database
- `install`: Compute and install the Smack rules accordingly to the context
- `uninstall`: Remove any rule related to the what context describes

Installing involves the following steps:

![Figure: install smack](assets/SEC-LSM-MGR.fig-smack-install.svg)

The operation are:

- **create smack rules**: create the file `/etc/smack/accesses.d/<ID>.smack` that
  contains the Smack rules for the application of id `<ID>`. Use the template
  file `/usr/share/sec-lsm-manager/app-template.smack`

- **load smack rules**: add the created rules of the created file into the living
  rule set of the kernel.

- **install plugs**: create symbolic links of plugs and set them label

- **label paths**: set the smack label for each path of the context

When installation fails, it an uninstallation is performed for cleaning.

The uninstallation involves the following steps:

![Figure: uninstall smack](assets/SEC-LSM-MGR.fig-smack-uninstall.svg)

The operation are:

- **unlabel paths**: set common label to paths of the context

- **uninstall plugs**: remove symbolic links of plugs

- **unload smack rules**: remove rules of `/etc/smack/accesses.d/<ID>.smack`
  from kernel's live rules

- **remove smack rules**: remove the file of rules `/etc/smack/accesses.d/<ID>.smack`

