# High level test cases of redpesk-core/sec-lsm-manager

.VERSION: DRAFT

.AUTHOR: José Bollo [IoT.bzh]

.AUDIENCE: ENGINEERING

.DIFFUSION: CONFIDENTIAL

.git-id($Id$)

The component redpesk-core/sec-lsm-manager is here denoted as
SEC-LSM-MANAGER

This document list the tests ensuring that high level requirements
of SEC-LSM-MANAGER (@SEC-LSM-MGR.HRQ) are fulfilled.

## Tests

### Service is installed

.TEST-CASE SEC-LSM-MGR.HTC-T-SER-INS

Check that the service is installed.

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-SYS-SER

.PROCEDURE

1. The command `systemctl status sec-lsm-manager` should show that the service
exists and is running.

### Service starts automatically

.TEST-CASE SEC-LSM-MGR.HTC-T-SER-STA-AUT

Check that the service starts automatically.

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-SYS-SER

.PRECONDITION

.PROCEDURE

1. The command `sec-lsm-manager-cmd log` must output `logging off` or
`logging on` instead of prompting `Connection refused`

### Service stops automatically

.TEST-CASE SEC-LSM-MGR.HTC-T-SER-STO-AUT

Check that the service automatically stops after the defined period.

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-SYS-SER

.PRECONDITION

.PROCEDURE

1. Check that *sec-lsm-manager* isn't running using `ps -e | grep sec-lsm-manager`
2. Activate the sec-lsm-manager using `sec-lsm-manager-cmd log`
3. Loop on checking that *sec-lsm-manager* is now running using
   `ps -e | grep sec-lsm-manager`
4. Note how long time it took to shutdown and check that it does not differ from
   the expected value with more than 5%
5. Another method is to invoke `time sec-lsm-managerd -k -S /tmp -s 10`
   and check that the reported real time is 10 seconds.

### Service socket is protected

.TEST-CASE SEC-LSM-MGR.HTC-T-SER-SOC-PRO

Check that the server socket can only be accessed by clients of the group sec-lsm-manager.

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-PRO-SOC

.PRECONDITION

.PROCEDURE

1. Check that the file `/run/sec-lsm-manager.socket`
   - is a socket file
   - is owned by the user `sec-lsm-manager`
   - is in the group `sec-lsm-manager`
   - is readable and writable by the user and the group
   - is neither readable nor writable nor executable by others

2. Check that a process not in the group `sec-lsm-manager` can't connect
   to the socket `/run/sec-lsm-manager.socket`. It can be done, by
   example, using the utility `socat` as in the the command:
   `echo sec-lsm-manager 1 |
    sudo -u guest socat stdio unix-client:/run/sec-lsm-manager.socket`

### Disconnect on protocol violation

.TEST-CASE SEC-LSM-MGR.HTC-T-DIS-PRO-VIO

Check that a violation of the protocol leads to an immediate
disconnection.

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-DIS-PRO-VIO

.PRECONDITION

.PROCEDURE

1. Connect to SEC-LSM-MANAGER using command
   `socat stdio unix-client:/var/run/sec-lsm-manager.socket`
2. Enter `sec-lsm-manager 1`
3. Check it returns `done 1`
4. Enter `turlututu`
5. Check that the connection is closed and that socat gives up
   Before closing, the SEC-LSM-MANAGER can report `error invalid`

### Error state

.TEST-CASE SEC-LSM-MGR.HTC-T-ERR-STA

Check that when an error has been reported, all settings and actions
except `clear` are returning `error not-recoverable`

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-ERR-STA

.PRECONDITION

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd -ek id x id xx path / x 
   permission x plug / x / install uninstall clear id xx`
2. Check that it outputs `error not-recoverable` where expected:
   for the commands from `id xx` to `uninstall`
3. Repeat the previous steps but replace the error generation `id x`
   with errors linked to an other setting: `path / x`, `permission x`,
   `plug / x /`.

### Setting of id properties

.TEST-CASE SEC-LSM-MGR.HTC-T-SET-ID-PRO

Check that id property is correctly recorded in the context

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-VAL-ID-QUE

.PRECONDITION

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd id toto display`
2. Check that it correctly reported the sent id property

### Check of identifier validity

.TEST-CASE SEC-LSM-MGR.HTC-T-CHE-IDE-VAL

Check that querying an invalid id return an error

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-VAL-APP-IDE

.PRECONDITION

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd id abc-ABC_123` and check it that
   does not return an error but a valid acknowledge
2. Run the command `sec-lsm-manager-cmd id $(head -c 200 /dev/zero | tr '\0' x)`
   and check it that does not return an error but a valid acknowledge
3. Run the command `sec-lsm-manager-cmd id a` and check it that
   returns an error (too small)
4. Run the command `sec-lsm-manager-cmd id aa@` and check it that
   returns an error (bad character)
5. Run the command `sec-lsm-manager-cmd id $(head -c 201 /dev/zero | tr '\0' x)`
   and check it that returns an error

### No second id property

.TEST-CASE SEC-LSM-MGR.HTC-T-NO-SEC-ID-PRO

Check that it is an error to query a id property if the id property
has already be given

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-QUE-ID-ONL-ONC

.PRECONDITION

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd id toto id titi`
2. Check that it returned a success status first and then and error
   indicating that the path is already set

### Setting of path properties

.TEST-CASE SEC-LSM-MGR.HTC-T-SET-PAT-PRO

Check that path properties are correctly recorded in the context

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-VAL-PAT-QUE

.PRECONDITION

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd path /tmp data path /etc conf display`
2. Check that it correctly reported the 2 sent paths properties

### Check path existence of path properties

.TEST-CASE SEC-LSM-MGR.HTC-T-CHE-PAT-EXI-PAT-PRO

Check that setting the path property on an inexistng file returns an error
but that querying an existing path doesn't

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-VAL-PAT

.PRECONDITION

.PROCEDURE

1. Check that the file `/TOTO` does not exist
2. Run the command `sec-lsm-manager-cmd path /TOTO default` and check it that
   returns an error
3. Check that the file `/tmp` exists
4. Run the command `sec-lsm-manager-cmd path /tmp default` and check it that
   does not return an error but a valid acknowledge

### Check path type validity of path properties

.TEST-CASE SEC-LSM-MGR.HTC-T-CHE-PAT-TYP-VAL-PAT-PRO

Check that querying an invalid path type in a path property returns an error
but that querying a valid type doesn't

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-VAL-PAT-TYP

.PRECONDITION

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd path /tmp toto` and check it that
   returns an error
2. Run the command `sec-lsm-manager-cmd path /tmp data` and check it that
   does not return an error but a valid acknowledge

### No path duplication

.TEST-CASE SEC-LSM-MGR.HTC-T-NO-PAT-DUP

Check that it is an error to add a second path property for
a path already given

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-ONL-ONC-PER-PAT

.PRECONDITION

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd path /tmp data path /tmp lib` and
   check that it returned a success status first and then and error
   indicating that the path is already set

### Setting permission properties

.TEST-CASE SEC-LSM-MGR.HTC-T-SET-PER-PRO

Check that permission property is correctly recorded in the context

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-VAL-PER-QUE

.PRECONDITION

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd permission p1 permission p2 display`
2. Check that it correctly reported the sent permission properties

### No permission duplication

.TEST-CASE SEC-LSM-MGR.HTC-T-NO-PER-DUP

Check that it is an error to add a second permission property for
a permission already given.

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-QUE-PER-ONL-ONC

.PRECONDITION

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd permission p1 permission p1` and
   check that it returned a success status first and then and error
   indicating that the permission is already set

### Check permission validity

.TEST-CASE SEC-LSM-MGR.HTC-T-CHE-PER-VAL

Check that querying an invalid permission in a permission property returns an error
but that querying a valid doesn't

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-VAL-APP-PER

.PRECONDITION

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd permission p1` and check it that
   and check it that does not return an error but a valid acknowledge
2. Run the command `sec-lsm-manager-cmd permission $(head -c 1024 /dev/zero | tr '\0' x)`
   and check it that does not return an error but a valid acknowledge
3. Run the command `sec-lsm-manager-cmd permission p` and check it that
   returns an error (too small)
4. Run the command `sec-lsm-manager-cmd permission $(head -c 1025 /dev/zero | tr '\0' x)`
   and check it that returns an error

### Setting plug properties

.TEST-CASE SEC-LSM-MGR.HTC-T-SET-PLU-PRO

.TYPE integration
Check that plug property is correctly recorded in the context

.PRECONDITION

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-VAL-PLU-QUE

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd plug / id / display`
2. Check that it correctly reported the sent id property

### Check plug validity

.TEST-CASE SEC-LSM-MGR.HTC-T-CHE-PLU-VAL

.TYPE integration
Check that validity of plug arguments property is correctly checked

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-VAL-DIR

.PRECONDITION

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd plug /nowhere id /` and check
   it detects that `/nowhere` doesn't exists
2. Run the command `sec-lsm-manager-cmd plug / x /` and check
   it detects that `x` is invalid
3. Run the command `sec-lsm-manager-cmd plug / id /nowhere` and check
   it detects that `/nowhere` doesn't exists

### Querying context

.TEST-CASE SEC-LSM-MGR.HTC-T-QUE-CON

Check that querying the context reports correctly the properties
already set since previous clear.

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-VAL-DIS-QUE

.PRECONDITION

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd display` and check it
   displays nothing
2. Run the command `sec-lsm-manager-cmd id xx display` and check it
   displays `id xx`
3. Run the command `sec-lsm-manager-cmd path / default display` and check it
   displays `path / default`
4. Run the command `sec-lsm-manager-cmd permission xx display` and check it
   displays `permission xx`
5. Run the command `sec-lsm-manager-cmd plug / xx / display` and check it
   displays `plug / xx /`

### Clearing context

.TEST-CASE SEC-LSM-MGR.HTC-T-CLE-CON

Check that querying the clear of the context removes all set properties.

.TYPE integration

.REQUIRED-BY @SEC-LSM-MGR.HRQ-R-CON-PRO
.REQUIRED-BY @SEC-LSM-MGR.PRO-U-VAL-CLE-QUE

.PRECONDITION

.PROCEDURE

1. Run the command `sec-lsm-manager-cmd -ek id xx path / default
   permission xx plug / xx / display clear display` and check that display
   doesn't report any property.



