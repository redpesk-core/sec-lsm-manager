# Developer user manual of redpesk-core/sec-lsm-manager


## Development of SEC-LSM-MANAGER

The SEC-LSM-MANAGER must be coded accordingly to the rules IOT-DEV-PROFILE-SYSTEM
(see SEC-LSM-MGR.REQ-9)

**MOTIVATION**: The service SEC-LSM-MANAGER is a sensitive service and
it must conform to quality and reliability practices in development.





## Analysis of required capabilities


### SEC-LSM-MGR.ADM-CAP

CAP\_MAC\_ADMIN
CAP\_DAC\_OVERRIDE
CAP\_MAC\_OVERRIDE
CAP\_SYS\_ADMIN
CAP\_DAC\_READ\_SEARCH
CAP\_SETFCAP
CAP\_FOWNER


### SEC-LSM-MGR.ADM-USRGRP

user: sec-lsm-manager
group: sec-lsm-manager
supplementatry groups: cynagora

## development package

See developer guide

```
/usr/include/sec-lsm-manager.h
/usr/lib/libsec-lsm-manager.so
/usr/lib/pkgconfig/sec-lsm-manager.pc
```


## manual interaction with SEC-LSM-MANAGER

It is possible to interact with the security manager if you have root
access to some console.

All the interaction using either `sec-lsm-manager-cmd` or `socat`
are mostly intended for debuging and should not happen in normal use.

### using sec-lsm-manager-cmd

The command `sec-lsm-manager-cmd` can be used 


### using socat

If you master the protocol, it is possible to use the "swiss army knife"
tool `socat` as shows the below example:

```
$ socat stdio unix-client:/var/run/sec-lsm-manager.socket
sec-lsm-manager 1
done 1
log
done off
log on
done on
display
done
id zorro
done
path zorro lib
done
permission zorro
done
display
string id zorro
string path zorro lib
string permission zorro
done
install
error sec_lsm_manager_handle_install
display
string error on
string id zorro
string path zorro lib
string permission zorro
done
clear
done
display
done
log off
done off
```

