# High level test procedure of redpesk-core/sec-lsm-manager

.VERSION: DRAFT

.AUTHOR: José Bollo [IoT.bzh]

The component redpesk-core/sec-lsm-manager is here denoted as
SEC-LSM-MANAGER

This document list the tests ensuring that high level requirements
of SEC-LSM-MANAGER (@SEC-LSM-MGR.HRQ) are fulfilled.

## Procedure

.PROCEDURE
The command `systemctl status sec-lsm-manager` should show that the service
exists and is running.

### Service starts automatically

.TEST-CASE SEC-LSM-MGR.HTP-T-SER-STA-AUT
Check that the service starts automatically.

.REQUIRED-BY 

.AFTER

.PROCEDURE
The command `sec-lsm-manager-cmd log` must output `logging off` instead of
prompting `Connection refused`


















### Invocation for help 

.TEST-CASE SEC-LSM-MGR.HTP-T-INV-HEL
Invoke SEC-LSM-MANAGER with argument `--help` and check that it prints a short help.

.REQUIRED-BY

### Invocation for version 

.TEST-CASE SEC-LSM-MGR.HTP-T-INV-VER
Invoke SEC-LSM-MANAGER with argument `--version` and check that it prints its version.

.REQUIRED-BY

### Invocation with no version and no help

.TEST-CASE SEC-LSM-MGR.HTP-T-INV-WIT-NO-VER-NO-HEL
Invoke SEC-LSM-MANAGER without argument `--version` and without argument `--help`
and check that it does not exits until killed.

.REQUIRED-BY

### Use only UDS sockets

.TEST-CASE SEC-LSM-MGR.HTP-T-USE-ONL-UDS-SOC
Invoke SEC-LSM-MANAGER without argument `--version` and without argument `--help`
and check that it only opens UDS socket by checking its opened files using `lsof`
tool.

.REQUIRED-BY

### 

### Test


## Protocol, presentation layer

Tests of the presentation layer are using the below table:

    # stream                               count field1  field2    field3    field4   field5
    - ------------------------------------ ----- ------- --------- --------- -------- ---------
    1 "\n"                                   0
    2 "first second third forth fifth\n"     5   "first" "second"  "third"    "forth" "fifth"
    3 " second third  fifth\n"               5   ""      "second"  "third"    ""      "fifth"
    4 "\  sec\ond th\\nird \  fi\ fth\\\n"   5   " "     "sec\ond" "th\nird"  " "     "fi fth\"
    - ------------------------------------ ----- ------- --------- --------- -------- ---------

The sample 1 is an empty record.

The sample 2 shows fields that are said "normal" because
not having special character having to be ecaped.

The sample 3 shows the case of empty fields. A field
is empty because its value has no character.

The sample 4 shows the several cases of escaping or not
for fields said "strange" because having to be escaped
(and let be honest not expected to be useful).

### Encoding normal fields

.TEST-CASE SEC-LSM-MGR.HTP-T-ENC-NOR-FIE
Normal fields are encoded without escaping, using sample 2
fields and expecting sample 2 stream.

.REQUIRED-BY

### Decoding normal fields

.TEST-CASE SEC-LSM-MGR.HTP-T-DEC-NOR-FIE
Normal fields are correctly decoded from input stream, using
sample 2 stream and expecting sample 2 fields.

.REQUIRED-BY

### Encoding special fields

.TEST-CASE SEC-LSM-MGR.HTP-T-ENC-SPE-FIE
Special fields are encoded with correct escaping, using sample 4
fields and expecting sample 4 stream.

.REQUIRED-BY @SEC-LSM-MGR.LRQ-R-PRE-LAY

### Decoding special fields

.TEST-CASE SEC-LSM-MGR.HTP-T-DEC-SPE-FIE
Special fields are correctly unescaped from input stream, using
sample 4 stream and expecting sample 4 fields.

.REQUIRED-BY @SEC-LSM-MGR.LRQ-R-PRE-LAY

### Encoding empty fields

.TEST-CASE SEC-LSM-MGR.HTP-T-ENC-EMP-FIE
Empty fields are correctly encoded, using sample 3
fields and expecting sample 3 stream.

.REQUIRED-BY @SEC-LSM-MGR.LRQ-R-PRE-LAY

### Decoding empty fields

.TEST-CASE SEC-LSM-MGR.HTP-T-DEC-EMP-FIE
Empty fields are correctly extracted from input stream, using
sample 3 stream and expecting sample 3 fields.

.REQUIRED-BY @SEC-LSM-MGR.LRQ-R-PRE-LAY

### Transmitting zero field

.TEST-CASE SEC-LSM-MGR.HTP-T-TRA-ZER-FIE
Zero field are correctly encoded, using sample 1
fields and expecting sample 1 stream.

.REQUIRED-BY @SEC-LSM-MGR.LRQ-R-PRE-LAY

### Receiving fields

.TEST-CASE SEC-LSM-MGR.HTP-T-REC-FIE
Zero fields are correctly extracted from input stream, using
sample 4 stream and expecting sample 4 fields.

.REQUIRED-BY @SEC-LSM-MGR.LRQ-R-PRE-LAY

