## SMACK

SMACK (Simplified Mandatory Access Control Kernel) allows to define simple rules to limit the rights of a user or a process.

### Context

The context of the actual user is defined in the file :

```bash
/proc/$$/attr/current
```

### Rules

A rule is defined as follows :

```bash
System  User  rwxat
```

- System is the subject's label
- User is the object's label
- rwx are the access read, write, execute, append and transmute

> ℹ️ The processes System can read, write and execute User objects.


#### Possible access

- r = read
- w = write
- x = execute
- t = transmute (If a file is created in a directory with transmute access, it will inherit of the label of parent)
- a = append (add)
- l = lock (use for locking a file ==> Read-Only)
- b = bring-up (equivalent of permissive mode)


Without any capability, you can only **reduce** acesses.
If you want to change a rule you need capability **CAP_MAC_ADMIN**.

You have two ways to change rules :

1) Runtime

```bash
smackload subject object rwt
```

Changes are lost on restart.

2) Persistant

Create a file in `/etc/smack/accesses.d/` with rules :

```bash
# vim /etc/smack/accesses.d/demo-app.smack
subject object rwt
```

Changes are applied on restart.

#### Default smack access rules

|      | REQUESTED BY             | REQUESTED ON             |
| ---- | ------------------------ | ------------------------ |
| *    | 🛑 Access                 | ✔️ Access                |
| ^    | ✔️ Read or execute access |                          |
| _    |                          | ✔️ Read or execute access |

✔️ If subject and object have the same label

🛑 All other rules not explicitly defined


### Sources

https://www.kernel.org/doc/html/v4.15/admin-guide/LSM/Smack.html

https://wiki.tizen.org/Security:SmackThreeDomainModel
