# The Protocol of redpesk-core/sec-lsm-manager

.VERSION: DRAFT

.AUTHOR: José Bollo [IoT.bzh]

.AUDIENCE: ENGINEERING

.DIFFUSION: CONFIDENTIAL

.REVIEW: IREV1

.git-id($Id$)

The component redpesk-core/sec-lsm-manager is here denoted as
SEC-LSM-MANAGER

The document SEC-LSM-MGR.OVE describes SEC-LSM-MANAGER


## The protocol

The SEC-LSM-MANAGER protocol normalize the exchanges between
the SEC-LSM-MANAGER, here named as *the server*, and any process
requiring service of SEC-LSM-MANAGER, here named *the client*.

The protocol is oriented *line of text*.
Its implementation and description is splitted in two layers following
OSI layering standard: presentation and application.

The presentation layer decodes the received data to sets of fields,
hiding decoding details, providing one set for each received line.
Conversely it encodes sets of fields to data that can be transmitted,
hiding encoding details.

The application layer gives semantic meaning to received and emitted
set of keys.


## Presentation layer

### Encoding

.RULE SEC-LSM-MGR.PRO-U-ENC

The protocol is based on UTF8 encoded characters.
It is line oriented and each line is made of fields.

### Lines

.RULE SEC-LSM-MGR.PRO-U-LIN

A line is made of all the characters until the end-of-line character (LF) of
binary value 10.

### Fields

.RULE SEC-LSM-MGR.PRO-U-FIE

Within a line, the protocol distinguishes fields. The fields are separated by
exactly one space character (SPACE) of binary value 32.

```
STREAM ::= [ LINE ]...

LINE ::= [ FIELD [ "SPACE" FIELD ]... ] "LF"
```

### Escaping

.RULE SEC-LSM-MGR.PRO-U-ESC

Because SPACE or LF could be present in fields, the protocol use an escaping
character to hold it. The escaping character is the BACKSLASH (`\`) of binary
code 92. Each occurence of characters SPACE or LF must be escaped if present
in fields. Escaping the character BACKSLASH is (with itself) is needed only if
it is at end of a field or if it is followed in field by SPACE or LF because
when decoding received bytes, if the character BACKSLASH is not followed by SPACE,
LF or BACKSLASH (itself), it is taken litteraly. Though paranoiac emitter can
safely escape every occurencies of BACKSLASH.

```
FIELD ::= [ CHAR ]...

CHAR ::= "any character except SPACE, LF or BACKSLASH"
       | "BACKSLASH" [ "SPACE" | "LF" | "BACKSLASH" ]
```

Note that empty lines (line without field) and empy fields (field with no
character) are possible and permitted.

These rules are summarized on the below schema:

![Figure: presentation layer](assets/SEC-LSM-MGR.fig-protocol-presentation.svg)


## Application layer

Basically, the SEC-LSM-MANAGER protocol is a sequential query/reply
protocol. It means that when connected, the client sends queries and
for each query receives a reply (in the query order). In practice, the
server process completely each query before processing the nexts.

### Structure of queries and replies

Each query match one line of the transport and so one set of fields as
presented by the presentation layer. The first field of the field set
(line) is the key (**KEY**) of the query, the remaining fields are
the arguments (**ARG**).

For each query, there is one reply.

A reply is made of one or more field sets (lines). Similary to queries,
the first field is a key of the reply. The two keys `done` (**DONE**) and
`error` (**ERROR**) are terminating the reply. The key `done` indicates
a success in the execution of the query, conversely, the key `error`
indicates a failure. All the preceding field sets (lines) are
keyed data sets (and it is obvious that none of it is keyed by
**done** or **error**).

This can be summarized by the figure:

![Figure: structure](assets/SEC-LSM-MGR.fig-protocol-structure.svg)

The last field set carries the status of the query. That status
can have associated values.

### No spontaneous data

.RULE SEC-LSM-MGR.PRO-U-NO-SPO-DAT

The server SEC-LSM-MANAGER never sends unsolicited data.
The server always send data in reply to a query.

### Protocol violation

.RULE SEC-LSM-MGR.PRO-U-PRO-VIO

A violation of the protocol occurs in the following cases:

- a received field is not valid utf8;
- the keyword of the query is unknown;
- the count of parameter does not match the expected count;
- for LOG request if the argument is neither `on` nor `off`.

### Protocol violation report

.RULE SEC-LSM-MGR.PRO-U-PRO-VIO-REP

When a violation of the protocol is detected, the party detecting
the violation shall report it to its pair using the reply `error protocol`.

### Disconnection on protocol violation

.RULE SEC-LSM-MGR.PRO-U-DIS-PRO-VIO

After reporting a violation of the protocol, the party detecting the
violation shall disconnect unilaterally.

In other world, after sending `error protocol`, the connection
shall be closed.

### Client context

For each client, the SEC-LSM-MANAGER holds a client context.

At connection, the client context is empty (clear).

Depending on how they act on context, the queries can be grouped
in 4 kinds:

1. HELLO:   negociate the version of the protocol used for the context
2. INFO:    don't change the context
3. SETTING: change the context by setting properties
4. ACTION:  apply context for action or clear it

So, using these categories, a client interaction can be summarized by
the below figure:

![Figure: protocol overview](assets/SEC-LSM-MGR.fig-protocol-app-overview.svg)

### Context erased on disconnection

.RULE SEC-LSM-MGR.PRO-U-CON-ERA-DIS

When the client disconnects, its context is dropped to the trash and
can not be recovered in any way.

### Error state

.RULE SEC-LSM-MGR.PRO-U-ERR-STA

When a SETTING or an ACTION fails, noticed by returning an error status
in reply, the context shall enters in error state.
When a context is in error state, it refuses all settings
(SETTING) and all actions (ACTION) except CLEAR. If a refused
query is received, the reply is `error not-recoverable`.

![Figure: state protocol](assets/SEC-LSM-MGR.fig-protocol-states.svg)



### Summary of queries

The protocol defines the below queries:

- **HELLO**: negociation of the version of the protocol (HELLO)
- **LOG**: query or set the global log status of SEC-LSM-MANAGER (INFO)
- **DISPLAY**: list the current context properties (INFO)
- **ID**: set the application identifier (SETTING)
- **PATH**: add a path property (SETTING)
- **PERMISSION**: add a permission property (SETTING)
- **PLUG**: add a plug property (SETTING) 
- **CLEAR**: reset the context, remove properties, clear error (ACTION)
- **INSTALL**: install security based on current properties (ACTION)
- **UNINSTALL**: remove security based on current properties (ACTION)

![Figure: queries](assets/SEC-LSM-MGR.fig-protocol-queries.svg)



### HELLO at connection

The query HELLO serves the purpose of negociating a version of the protocol.

synopsis:

![Figure: HELLO](assets/SEC-LSM-MGR.fig-protocol-hello.svg)

The client offers a list of protocol versions it can use
(today version 1 only) in the preferred order. The server
answers `done` with the value of the acknowledged version it
will use. That value is in the list presented by the query.

#### HELLO shall be the first query

.RULE SEC-LSM-MGR.PRO-U-HEL-SHA-BE-FIR-QUE

The hello query shall be the first query that present the client.
If the client send an HELLO query after an other query, it is an
error and a protocol violation.

#### Server return the version to use

.RULE SEC-LSM-MGR.PRO-U-SER-RET-VER-USE

The server scans in order the versions offered by the client and
choose to use the first version it supports. The reply values is that
choosen version.

#### Disconnection if unsupported versions

.RULE SEC-LSM-MGR.PRO-U-DIS-IF-UNS-VER

If none of the offered protocol versions is supported, the server
replies *error protocol* and disconnects.

In other words, unsupported versions in HELLO are treated as protocol
violations.

#### HELLO is optional

.RULE SEC-LSM-MGR.PRO-U-HEL-OPT

Negociating the protocol version at cannection is optionnal.
When no HELLO annouce is made, the default version of the protocol
is the latest available version: 1.

The reason why the latest is used is because the protocol is intended
be backward compatible.



### LOG: Getting and setting logging

The query LOG is administrative. It can be used to enforce the
SEC-LSM-MANAGER to log all traffic. This might be useful for
debugging or for auditing.

That query never fail.

synopsis:

![Figure: LOG](assets/SEC-LSM-MGR.fig-protocol-log.svg)

This query doesn't change the context of the client but it can
change the global state of the server SEC-LSM-MANAGER.

Without argument, it simply queries the current state of logging.

With an argument, it set on or off the logging.

In all cases, it returns the final state of logging.

#### Valid LOG query

.RULE SEC-LSM-MGR.PRO-U-VAL-LOG-QUE

A valid LOG query has either no argument or exactly
one argument that is either `on` or `off`.

#### Valid LOG reply

.RULE SEC-LSM-MGR.PRO-U-VAL-LOG-REP

A LOG query never fails and the reply always have one value that
is either `on` if logging is currently on or `off` if logging is
currently off.



### DISPLAY: Listing the context

The query DISPLAY sends to the client its current context. That
behaviour can be used for debugging and for reporting
messages on failure.

synopsis:

![Figure: DISPLAY](assets/SEC-LSM-MGR.fig-protocol-display.svg)

Received data are keyed with `string` then the next fields are
corresponding to the received queries except for `error`.

The error status is only sent when the context is in error state.

#### Valid DISPLAY query

.RULE SEC-LSM-MGR.PRO-U-VAL-DIS-QUE

A valid DISPLAY query has no argument.

#### Valid DISPLAY reply

.RULE SEC-LSM-MGR.PRO-U-VAL-DIS-REP

A DISPLAY queries shall normally not fail but because it can encounter
network issue during the display of properties, it can report an error.

The reply to a DISPLAY query is made of as many field sets (lines)
as needed, all of it having the key `string` with values except the
last one that has the key `done` without value.

The valid values for the key `string` are:

- the queries of of previous property settings except CLEAR
  i.e. the queries ID, PATH, PERMISSION and PLUG

- the values `error` `on` if and only if the context has entered
  in error state and needs a CLEAR

In case of error during display of properties, the server
replies `error internal` and enters in error state.


### ID: Setting the application identifier

The query ID set the identifier of the application in the current client context.

synopsis:

![Figure: ID](assets/SEC-LSM-MGR.fig-protocol-id.svg)

Set the ID of the application in the context.

Setting the application identifier is not mandatory. Some properties may not
require it. For version 1 of the protocol, only the property *path* with the
type *default* does not require identifier.

The figure below shows that rule for installation/uninstallation.

![Figure: state according to ID](assets/SEC-LSM-MGR.fig-install-states.svg)

#### Valid ID query

.RULE SEC-LSM-MGR.PRO-U-VAL-ID-QUE

A valid ID query has one argument, the identifier of the application
that must be valid.

#### Valid application identifier

.RULE SEC-LSM-MGR.PRO-U-VAL-APP-IDE

A valid application identifier must match the regular expression

```
[-_a-zA-Z0-9]{2,200}
```

#### Query ID only once

.RULE SEC-LSM-MGR.PRO-U-QUE-ID-ONL-ONC

It is an error to query setting the application identifier if the application
identifier has already been set.

#### Valid ID reply

On success, a ID reply has no value.

On error, a ID reply has one of the below error indicator
and enters in error state:

- invalid: the identifier is invalid
- already-set: the identifier is already set in the context
- not-recoverable: the context has been in error state
- internal: internal server error



### PATH: Adding a path

The query PATH adds a path property telling the expected security type
of an item of the filesystem (file, link or directory).

synopsis:

![Figure: PATH](assets/SEC-LSM-MGR.fig-protocol-path.svg)

Add the file system entry of PATH with the given PATH-TYPE in the context.

#### Valid PATH query

.RULE SEC-LSM-MGR.PRO-U-VAL-PAT-QUE

A valid PATH query has 2 arguments, the path, *PATH*, to the entry in the filesystem
and its type, *PATH-TYPE*.

#### Validity of the path

.RULE SEC-LSM-MGR.PRO-U-VAL-PAT

A valid path is the path to an existing entry in the filesystem and the size
of the path is not bigger than 1024 bytes.

#### Validity of the path type

.RULE SEC-LSM-MGR.PRO-U-VAL-PAT-TYP

The valid values for PATH-TYPE are:

- **default**: System wide default access. No write access is allowed but files
  can be read and executed.
- **conf**: Configuration file of a managed application. The application can
  access it in read only mode. Other applications are not allowed to access it.
- **data**: Data file of a managed application. The application can
  access it in read only mode. Other applications are not allowed to access it.
- **exec**: Executable file of a managed application. The application can
  access it in read only mode and can execute it. Other applications are not
  allowed to access it.
- **http**: Served file of a managed application. The application can
  access it in read only mode. Other applications are not allowed to access it.
- **icon**: Icon file of a managed application. The application can
  access it in read only mode. Other applications are not allowed to access it.
- **id**: Root directory of a managed application. The application can
  access it in read/write/execute mode.  Other applications are not
  allowed to access it.
- **lib**: Library file of a managed application. The application can
  access it in read only mode and can execute it. Other applications are not
  allowed to access it.
- **plug**: Exported directory of a managed application. The content of this
  directory is exported to some selected other applications that can access
  its content.
- **public**: Public file of a managed application. Any application can access
  it for read/write/execute.

#### Only once per path

.RULE SEC-LSM-MGR.PRO-U-ONL-ONC-PER-PAT

The path property of a given path shall be set only one time. Trying to set
the path property of an already set path shall lead to an error.

#### Valid PATH reply

.RULE SEC-LSM-MGR.PRO-U-VAL-PAT-REP

On success, a PATH reply has no value.

On error, a PATH reply has one of the below error indicator
and enters in error state:

- invalid: the path or its path-type is invalid
- already-set: that path is already set in the context
- not-found: the path doesn't exist
- no-access: the path can't be accessed
- not-recoverable: the context has been in error state
- internal: internal server error



### PERMISSION: Adding a permission

The PERMISSION query adds a property permission telling to grant a permission
to the application.

synopsis:

![Figure: PERMISSION](assets/SEC-LSM-MGR.fig-protocol-permission.svg)

Add the PERMISSION in the context.

#### Valid PERMISSION query

.RULE SEC-LSM-MGR.PRO-U-VAL-PER-QUE

A valid PERMISSION query has one argument, the permission
that must be valid.

#### Valid application permission

.RULE SEC-LSM-MGR.PRO-U-VAL-APP-PER

A valid permission is a string of at least 2 bytes and at most 1024 bytes.

#### Query PERMISSION only once

.RULE SEC-LSM-MGR.PRO-U-QUE-PER-ONL-ONC

It is an error to query setting a permission if that permission has already
been set.

#### Valid PERMISSION reply

.RULE SEC-LSM-MGR.PRO-U-VAL-PER-REP

On success, a PERMISSION reply has no value.

On error, a PERMISSION reply has one of the below error indicator
and enters in error state:

- invalid: the permision is invalid
- already-set: that permision is already set in the context
- not-recoverable: the context has been in error state
- internal: internal server error




### PLUG: Adding a plug

The PLUG query adds a property telling to plug an exported directory in
an importing directory and to allow the access to the exported directory
content to an application.

synopsis:

![Figure: PLUG](assets/SEC-LSM-MGR.fig-protocol-plug.svg)

Plugs the directory of path EXPORTED in the directory of path IMPORT
and gives access to its content to the application of APPID.

Note that the application APPID should have access to IMPORT the directory
of importation.

#### Valid PLUG query

.RULE SEC-LSM-MGR.PRO-U-VAL-PLU-QUE

A valid PLUG query has 3 arguments:

1. the path of the exported directory to plug
2. the identifier of the application allowed to access the plugged directory,
   shall be a valid application identifier
3. the path of the importation directory

#### Validity of directories

.RULE SEC-LSM-MGR.PRO-U-VAL-DIR

The entries EXPORTED and IMPORT must exist exist in the filesytem, must be
directories and the size of each path is not bigger than 1024 bytes.

#### Only once per import directory

.RULE SEC-LSM-MGR.PRO-U-ONL-ONC-PER-IMP-DIR

The IMPORT directory shall be set only one time. Trying to set
the plug property lead to an error if an already existing plug property
exists with a same IMPORT path.

#### Valid PLUG reply

.RULE SEC-LSM-MGR.PRO-U-VAL-PLU-REP

On success, a PLUG reply has no value.

On error, a PLUG reply has one of the below error indicator
and enters in error state:

- invalid: an argument is invalid
- already-set: that IMPORT already has a plug in the context
- not-found: one of the directories doesn't exist
- no-access: one of the directories can't be accessed
- not-dir: one of the paths is not a directory
- not-recoverable: the context has been in error state
- internal: internal server error



### CLEAR: Reseting the context

The query CLEAR removes all properties of the context and clears
the error state.

That query never fail.

synopsis:

![Figure: CLEAR](assets/SEC-LSM-MGR.fig-protocol-clear.svg)

The client asks SEC-LSM-MANAGER to reset the state of the context to its
original state as if connection just occured.

#### Valid CLEAR query

.RULE SEC-LSM-MGR.PRO-U-VAL-CLE-QUE

A valid CLEAR query has no argument.

#### Valid CLEAR reply

.RULE SEC-LSM-MGR.PRO-U-VAL-CLE-REP

CLEAR query never fails and alway reply done without value.



### INSTALL: Query installation

The INSTALL query expect the SEC-LSM-MANAGER to install the security
policy as defined by the current context.

synopsis:

![Figure: INSTALL](assets/SEC-LSM-MGR.fig-protocol-install.svg)

Install an application based on the properties of the context.

#### Valid INSTALL query

.RULE SEC-LSM-MGR.PRO-U-VAL-INS-QUE

The valid INSTALL query has no argument.

#### Valid INSTALL reply

.RULE SEC-LSM-MGR.PRO-U-VAL-INS-REP

On success, an INSTALL reply has no value.

On error, an INSTALL reply has one of the below error indicator
and enters in error state:

- invalid: the application identifier is missing
- forbidden: no permission to install plugin
- not-recoverable: the context has been in error state
- internal: internal server error



### UNINSTALL: Query uninstallation

The UNINSTALL query expect the SEC-LSM-MANAGER to remove the security
policy that was installed accordingly to the current context.

synopsis:

![Figure: UNINSTALL](assets/SEC-LSM-MGR.fig-protocol-uninstall.svg)

Uninstall an application based on the properties of the context.

#### Valid UNINSTALL query

.RULE SEC-LSM-MGR.PRO-U-VAL-UNI-QUE

The valid UNINSTALL query has no argument.

#### Valid UNINSTALL reply

.RULE SEC-LSM-MGR.PRO-U-VAL-UNI-REP

On success, an UNINSTALL reply has no value.

On error, an UNINSTALL reply has one of the below error indicator
and enters in error state:

- invalid: the application identifier is missing
- not-recoverable: the context has been in error state
- internal: internal server error


## Protocol summary view

![Figure: protocol summary](assets/SEC-LSM-MGR.fig-protocol-grammar.svg)


