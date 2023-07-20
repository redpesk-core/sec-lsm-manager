# The Protocol of SEC-LSM-MANAGER

## Introduction

The protocol is UTF-8 line oriented. Basically, the client sends one line that forms
a request and receive one line that forms the reply.

A line is made of all the characters until the end-of-line character (LF) of
binary value 10.

Within a line, the protocol distinguishes fields. The fields are separated by space
character (SPACE) of binary value 32.

Because SPACE or LF could be present in fields, the protocol use an escaping
character to hold it. The escaping character is the backslash (\) of binary code 92.
The 3 characters SPACE, LF and ESCAPE must be escaped if present in fields.
Escaping these characters means inserting before it the escaping character backslash.

```
          LINE ::= FIELD [ SPACE FILED ]... LF

         FIELD ::= [ FIELD-CHAR ]...

    FIELD-CHAR ::= UNESCAPED-CHAR
                 | ESCAPED-CHAR

UNESCAPED-CHAR ::= ANY CHARACTER EXCEPT SPACE OR LF OR ESCAPED-CHAR

  ESCAPED-CHAR ::= BACKSLASH LF
                 | BACKSLASH SPACE
                 | BACKSLASH BACKSLASH
```

In all circumstances, the server SEC-LSM-MANAGER is allowed to close the connection.

### Normal replies and error replies

Normal replies are indicating that no error occured. Normal replies
are generally made of a single status line but it may also include, before
the status line, several lines of data.

On normal reply, the status line has its first field equals to `done`

An error reply is made of a single status line whose first field equals to `error`.

```
       REPLY ::= NORMAL-REPLY | ERROR-REPLY

NORMAL-REPLY ::= [ LINE ]... DONE-LINE

 ERROR-REPLY ::= ERROR-LINE

   DONE-LINE ::= done [ ARGS ]...

  ERROR-LINE ::= error [ ARGS ]...
```

### Session

When a client connect, it establishes a unic and single session linked to the connection.
That session handles data related to one application. Action or queries are running in the
context of the current session and use or modify the current data of it.

When the client disconnect, its session is droped to the trash and can not be recovered
in any way.

### Notations

- c->s:    from client to sec-lsm-manager server
- s->c:    from sec-lsm-manager server to client
- [OPT]:   optional line
- [OPT\*]: optional line than can be repeated
- ID:      a string when the field is in ALL-CAPITAL-LETTERS


## Messages

### hello at connection

synopsis:

```
	c->s sec-lsm-manager 1
	s->c done 1
```

The client present itself with the version of the protocol it expects to
speak (today version 1 only). The server answer done with the acknowledged
version it will uses.

If hello is used, it must be the first message. If it is not used, the
protocol implicitely switch to the default version.

Later versions will accept more than one version, the server will choose
the one it supports and return it with done.

If the message is not understood or the version is not suported, the server
reply:

	s->c error invalid


### reseting the session state

synopsis:

	c->s clear
	s->c done

The client asks SEC-LSM-MANAGER to reset the state of the session to its
original state as if connection just occured.


### set the application identifier for the current session

synopsis:

```
	c->s id ID
	s->c done
```

Set the session's id to ID. Valid ids are strings of 2 characters or more but
not more than 199 characters that contains latin letters in upper or lower case,
arabic digits, dash or underscore (matching the regular expression `[-a-zA-Z\_0-9]{2,}`).

It is an error if the session's id is already set.
This can can be avoided by clearing the session before.


### add a file to the current session state

synopsis:

```
	c->s path PATH PATH-TYPE
	s->c done
```

Add the file of PATH with the given PATH-TYPE in the current session.

Valid PATH-TYPE are: default, conf, data, exec, http, icon, id, lib, public.

It is an error to add the same path a second time.

It is an error to put an invalid value for PATH-TYPE.


### add a permission to the current session state

synopsis:

```
	c->s permission PERMISSION
	s->c done
```

Add the permission in the current session.

It is an error to add the same permission a second time.


### install

synopsis:

```
	c->s install
	s->c done
```

Install an application with the current session data parameters.


### uninstall

synopsis:

```
	c->s uninstall
	s->c done
```

Uninstall an application with the current session data parameters.


### listing the session data

synopsis:

```
	c->s display
[OPT]	s->c string error on
[OPT]	s->c string id ID
[OPT*]	s->c string path PATH PATH-TYPE
[OPT*]	s->c string permission PERMISSION
	s->c done
```

Check whether the permission is granted (yes) or not granted (no)
or undecidable without querying an agent (ack).

This query ensure that the response is fast because agent are allowed to
delay requests before emitting the final status. But it doesn't ensure that
the answer is a final status. Receiving `ack` means that no final decision
can be decided. In that case the correct resolution is either to act as if
`no` were received or to ask for a check with not null probability that the
reply will take time.


### logging set/get

synopsis:

	c->s log [on|off]
	s->c done (on|off)

Tell to log or not the queries or query the current state.

With an argument, it activates or deactivates logging. Without argument,
it does nothing.

In all cases, returns the logging state afterward.

Logging is a global feature. The protocol commands that the server sends or
receives are printed to the journal or not.

