# SMACK

SMACK (Simplified Mandatory Access Control Kernel) allows to define simple rules to limit a user or a process rights.

## Context

The actual user context is defined in the following file :

```bash
/proc/$$/attr/current
```

## Rules

A rule is defined as follows :

```bash
System  User  rwxat
```

- System is the subject's label
- User is the object's label
- rwx are the access read, write, execute, append and transmute

> ℹ️ The processes System can read, write and execute User objects.

### Possible access

| Code | Meaning   |
| ---- | --------- |
| `r`  | read      |
| `w`  | write     |
| `x`  | execute   |
| `t`  | transmute |
| `a`  | append    |
| `l`  | lock      |
| `b`  | bring-up  |

- Transmute: a file created in a directory with transmute access will inherit the parent's label
- Lock: make a file read-only
- Bring-up: equivalent of permissive mode

Without any capability, you can only **reduce** accesses.
If you want to change a rule you need **CAP_MAC_ADMIN** capability.

You have two ways to change rules :

#### Runtime

```bash
smackload subject object rwt
```

Changes are lost on restart.

#### Persistent

Create a file in `/etc/smack/accesses.d/` with rules :

```bash
# vim /etc/smack/accesses.d/demo-app.smack
subject object rwt
```

Changes are applied on restart.

### Default smack access rules

|     | REQUESTED BY                | REQUESTED ON                |
| --- | --------------------------- | --------------------------- |
| *   | 🛑 Access                  | ✔️ Access                 |
| ^   | ✔️ Read or execute access |                             |
| _   |                             | ✔️ Read or execute access |

✔️ If subject and object have the same label

🛑 All other rules not explicitly defined

## Sources

- <https://www.kernel.org/doc/html/v4.15/admin-guide/LSM/Smack.html>
- <https://wiki.tizen.org/Security:SmackThreeDomainModel>
