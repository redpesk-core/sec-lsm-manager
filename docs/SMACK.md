# SMACK


SMACK (Simplified Mandatory Access Control Kernel) allows to define simple rules to limit the rights of a user or a process.

## Context

The context of the actual user is defined in the file :

```
/proc/$$/attr/current
```

## Rules

A rule is defined as follows:

```
System  User  rwxat
```

- System is the subject's label
- User is the object's label
- rwx are the access read, write, execute, append and transmute



With this rule :

> ℹ️ The processes System can read, write and execute User objects.



### Possible access

r = read

w = write

x = execute

t = transmute (If a file is created in a directory with transmute access, it will inherit of the label of parent)

a = append (add)

l = lock (use for locking a file ==> Read-Only)

b = bring-up (equivalent of permissive mode)




Without any capability, you can only **reduce** acesses. If you want to change a rule you need capability **CAP_MAC_ADMIN**

You have two ways to change rules :

- Edit files :

```
echo "subject object rwt" > /sys/fs/smackfs/load-self2
echo "subject object rwt" > /sys/fs/smackfs/load
```

- Use binary

```
smackload subject object rwt
```

In the old security manager an other way exists to add rules for specific application. You need to create a file in the repository :

```
/etc/smack/accesses.d
```


### Default smack access rules

|      | REQUESTED BY             | REQUESTED ON             |
| ---- | ------------------------ | ------------------------ |
| *    | 🛑 Access                 | ✔️ Access                 |
| ^    | ✔️ Read or execute access |                          |
| _    |                          | ✔️ Read or execute access |

✔️ If subject and object have the same label

🛑 All other rules not explicitly defined


Sources :

https://www.kernel.org/doc/html/v4.15/admin-guide/LSM/Smack.html

https://wiki.tizen.org/Security:SmackThreeDomainModel