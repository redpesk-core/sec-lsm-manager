#!/bin/bash

all_tests() {
	cat << END-OF-CAT

.TEST-CASE SEC-LSM-MGR.HTC-T-SER-INS
rpm -ql sec-lsm-manager | grep -v .build-id

.TEST-CASE SEC-LSM-MGR.HTC-T-SER-STA-AUT
sec-lsm-manager-cmd -e log

.TEST-CASE SEC-LSM-MGR.HTC-T-SER-SOC-PRO
ls -lZ /run/sec-lsm-manager.socket
sudo -u rp-owner sec-lsm-manager-cmd -e log

.TEST-CASE SEC-LSM-MGR.HTC-T-ERR-STA
sec-lsm-manager-cmd -ek id x id xx path / default permission xx plug / xx / install uninstall clear id xx
sec-lsm-manager-cmd -ek permission x id xx path / default permission xx plug / xx / install uninstall clear id xx
sec-lsm-manager-cmd -ek path / x id xx path / default permission xx plug / xx / install uninstall clear id xx
sec-lsm-manager-cmd -ek plug / x / id xx path / default permission xx plug / xx / install uninstall clear id xx

.TEST-CASE SEC-LSM-MGR.HTC-T-SET-ID-PRO
sec-lsm-manager-cmd -ek id toto display

.TEST-CASE SEC-LSM-MGR.HTC-T-CHE-IDE-VAL
sec-lsm-manager-cmd -ek id abc-ABC_123
sec-lsm-manager-cmd -ek id \$(head -c 200 /dev/zero | tr '\0' x) 
sec-lsm-manager-cmd -ek id a
sec-lsm-manager-cmd -ek id aa@
sec-lsm-manager-cmd -ek id \$(head -c 201 /dev/zero | tr '\0' x)

.TEST-CASE SEC-LSM-MGR.HTC-T-NO-SEC-ID-PRO
sec-lsm-manager-cmd -ek id toto id titi

.TEST-CASE SEC-LSM-MGR.HTC-T-SET-PAT-PRO
sec-lsm-manager-cmd -ek path /tmp data path /etc conf display

.TEST-CASE SEC-LSM-MGR.HTC-T-CHE-PAT-EXI-PAT-PRO
ls -ld /TOTO
ls -ld /tmp
sec-lsm-manager-cmd -ek path /TOTO default
sec-lsm-manager-cmd -ek path /tmp default

.TEST-CASE SEC-LSM-MGR.HTC-T-CHE-PAT-TYP-VAL-PAT-PRO
ls -ld /tmp
sec-lsm-manager-cmd -ek path /tmp toto
sec-lsm-manager-cmd -ek path /tmp data

.TEST-CASE SEC-LSM-MGR.HTC-T-NO-PAT-DUP
ls -ld /tmp
sec-lsm-manager-cmd -ek path /tmp data path /tmp lib

.TEST-CASE SEC-LSM-MGR.HTC-T-SET-PER-PRO
sec-lsm-manager-cmd -ek permission p1 permission p2 display

.TEST-CASE SEC-LSM-MGR.HTC-T-NO-PER-DUP
sec-lsm-manager-cmd -ek permission p1 permission p1

.TEST-CASE SEC-LSM-MGR.HTC-T-CHE-PER-VAL
sec-lsm-manager-cmd -ek permission p1
sec-lsm-manager-cmd -ek permission \$(head -c 1024 /dev/zero | tr '\0' x)
sec-lsm-manager-cmd -ek permission p
sec-lsm-manager-cmd -ek permission \$(head -c 1025 /dev/zero | tr '\0' x)

.TEST-CASE SEC-LSM-MGR.HTC-T-SET-PLU-PRO
sec-lsm-manager-cmd -ek plug / id / display

.TEST-CASE SEC-LSM-MGR.HTC-T-CHE-PLU-VAL
ls -ld /nowhere
sec-lsm-manager-cmd -ek plug /nowhere id /
sec-lsm-manager-cmd -ek plug / x /
sec-lsm-manager-cmd -ek plug / id /nowhere

.TEST-CASE SEC-LSM-MGR.HTC-T-QUE-CON
sec-lsm-manager-cmd -ek display
sec-lsm-manager-cmd -ek id xx display
sec-lsm-manager-cmd -ek path / default display
sec-lsm-manager-cmd -ek permission xx display
sec-lsm-manager-cmd -ek plug / xx / display

.TEST-CASE SEC-LSM-MGR.HTC-T-CLE-CON
sec-lsm-manager-cmd -ek id xx path / default permission xx plug / xx / display clear display

END-OF-CAT
}

tests=$(all_tests|awk '$1==".TEST-CASE"{print $2}')

get_test() {
	local name="$1"
	all_tests |
	awk -vtest="$name" '$1==".TEST-CASE" { p = $2==test; next } p'
}

CDPATH=
cd $(dirname $0)
for name in $tests
do
	echo $name ...
	eval "$(get_test $name)" > result/$name.result 2>&1
done

