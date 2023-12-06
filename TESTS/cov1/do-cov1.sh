#!/bin/bash

set -x

H=$(realpath $(dirname $0))
R=$(realpath $H/../..)
B=$R/build
PATH=$B/src:$PATH

error() {
	echo "$*" >&2
}

fatal() {
	error "$*"
	exit 1
}

getprog() {
	local name="$1"
	prog="$(type -p "$name")"
	if ! [[ -f "$prog" && -x "$prog" ]]
	then
		fatal "can't get program $name"
	fi
	echo -n "$prog"
}

rbk() {
	"$@" &
	echo "$!"
}

cynagorad=$(getprog cynagorad) || exit 1
cynadm=$(getprog cynagora-admin) || exit 1
seclsm=$(getprog sec-lsm-manager-smackd) || exit 1
cmd=$(getprog sec-lsm-manager-cmd) || exit 1
cmd="$cmd -e -k"

cynpid=
slmpid=

killdaemons() {
	[[ -n "$cynpid" ]] && kill -SIGKILL $cynpid
	[[ -n "$slmpid" ]] && kill -SIGINT $slmpid
}
trap killdaemons EXIT

# INVOKING
$seclsm --truc-machin
$seclsm --help
$seclsm --version
$seclsm -s 1 -k
$seclsm -k --user 100 --group 100 --groups 101,102,103 --shutoff 200 --socketdir //tohjer//fluck -M -O
$seclsm -k --user root --group root --groups root --shutoff 1 --socketdir /tohjer -M -O
$seclsm --user machin
$seclsm --group machin
$seclsm --groups machin
$seclsm --user root -g $(id -g)
$seclsm --socketdir $H///tmp/.//tmp// -M -O --group root
rm -r $H/tmp
$seclsm --groups root

# RUN CYNAGORA DAEMON
$cynagorad -l &
cynpid=$!
sleep 0.5

# TESTS
$B/src/tests/tests-smack

# RUN SEC-LSM-MANAGER DAEMON
$seclsm -k -l -s never -u $(id -u) -g $(id -g) -G $(id -G|tr ' ' ,)  &
slmpid=$!
sleep 0.5

# provision  session items

$cynadm set App:PUBLIC '*' '*' urn:redpesk:permission::public:plugs yes 1h

# SEC-LSM-MGR.HTC-T-ERR-STA
$cmd id x id xx path / default permission xx plug / xx / install uninstall clear id xx
$cmd permission x id xx path / default permission xx plug / xx / install uninstall clear id xx
$cmd path / x id xx path / default permission xx plug / xx / install uninstall clear id xx
$cmd plug / x / id xx path / default permission xx plug / xx / install uninstall clear id xx

# SEC-LSM-MGR.HTC-T-SET-ID-PRO
$cmd id toto display

# SEC-LSM-MGR.HTC-T-CHE-IDE-VAL
$cmd id abc-ABC_123
$cmd id $(head -c 200 /dev/zero | tr '\0' x) 
$cmd id a
$cmd id aa@
$cmd id $(head -c 201 /dev/zero | tr '\0' x)

# SEC-LSM-MGR.HTC-T-NO-SEC-ID-PRO
$cmd id toto id titi

# SEC-LSM-MGR.HTC-T-SET-PAT-PRO
$cmd path /tmp data path /etc conf display

# SEC-LSM-MGR.HTC-T-CHE-PAT-EXI-PAT-PRO
$cmd path /TOTO default
$cmd path /tmp default

# SEC-LSM-MGR.HTC-T-CHE-PAT-TYP-VAL-PAT-PRO
$cmd path /tmp toto
$cmd path /tmp data

# SEC-LSM-MGR.HTC-T-NO-PAT-DUP
$cmd path /tmp data path /tmp lib

# SEC-LSM-MGR.HTC-T-SET-PER-PRO
$cmd permission p1 permission p2 display

# SEC-LSM-MGR.HTC-T-NO-PER-DUP
$cmd permission p1 permission p1

# SEC-LSM-MGR.HTC-T-CHE-PER-VAL
$cmd permission p1
$cmd permission $(head -c 1024 /dev/zero | tr '\0' x)
$cmd permission p
$cmd permission $(head -c 1025 /dev/zero | tr '\0' x)

# SEC-LSM-MGR.HTC-T-SET-PLU-PRO
$cmd plug / id / display

# SEC-LSM-MGR.HTC-T-CHE-PLU-VAL
$cmd plug /nowhere id /
$cmd plug / x /
$cmd plug / id /nowhere

# SEC-LSM-MGR.HTC-T-QUE-CON
$cmd display
$cmd id xx display
$cmd path / default display
$cmd permission xx display
$cmd plug / xx / display

# SEC-LSM-MGR.HTC-T-CLE-CON
$cmd id xx path / default permission xx plug / xx / display clear display

# PLUG
$cmd plug / xx / display plug / yy /tmp display clear display
$cmd plug / xx / plug / yy /tmp plug / zz / display clear display

# UTF-8
$cmd id "$(printf '\x41\xde\x80\xee\x80\x80\xf6\x80\x80\x80')" display
$cmd id "$(printf '\xbf')" display
$cmd id "$(printf '\x80')" display
$cmd id "$(printf '\xde\x41')" display
$cmd id "$(printf '\xee\x41')" display
$cmd id "$(printf '\xee\x80\x41')" display
$cmd id "$(printf '\xee\xc0\x41')" display

# PATH
$cmd path '' default display
$cmd path //tmp//..//.///tmp// default display
$cmd path toto default display
$cmd path "/$(head -c 1023 /dev/zero | tr '\0' x)" default display
$cmd path "/$(head -c 1024 /dev/zero | tr '\0' x)" default display
$cmd plug x xx / display
$cmd plug / xx x display
$cmd plug "/$(head -c 1024 /dev/zero | tr '\0' x)" xx / display
$cmd plug / xx "/$(head -c 1024 /dev/zero | tr '\0' x)" display

# ESCAPING
$cmd permission 'to to' display
$cmd permission 'to\to' display
$cmd permission 'to
to' display

# LONG
$cmd permission $(head -c 1023 /dev/zero | tr '\0' v) \
	permission $(head -c 1023 /dev/zero | tr '\0' w) \
	permission $(head -c 1023 /dev/zero | tr '\0' x) \
	permission $(head -c 1023 /dev/zero | tr '\0' y) \
	permission $(head -c 1023 /dev/zero | tr '\0' z) \
	display

# LOG
$cmd log on id toto path / default permission xx display log off

# PROTOCOL
S=/home/jobol/.locenv/sec/var/run/sec-lsm-manager.socket
echo "fgbj,oij" | socat stdio unix-client:$S
echo "sec-lsm-manager 2 3 4" | socat stdio unix-client:$S
printf "sec-lsm-manager 1\ntoij\n"  | socat stdio unix-client:$S
printf "sec-lsm-manager 1\nlog tutu\n"  | socat stdio unix-client:$S

# INSTALL
T=$H/tmp
mkdir -p $T/{a,b,c}
touch $T/{a,b,c}/{x,y,z}
chmod +x $T/a/y
$cmd path $T/a/x default path $T/b default install uninstall
$cmd path $T/a/x conf install
$cmd path $T/a/x conf uninstall
$cmd id toto path $T/a/x conf path $T/a conf install uninstall
$cmd id toto path $T/a/x data path $T/a data install uninstall
$cmd id toto path $T/a/x exec path $T/a exec install uninstall
$cmd id toto path $T/a/y exec path $T/a exec install uninstall
$cmd id toto path $T/a/x http path $T/a http install uninstall
$cmd id toto path $T/a/x icon path $T/a icon install uninstall
$cmd id toto path $T/a/x id path $T/a id install uninstall
$cmd id toto path $T/a/x lib path $T/a lib install uninstall
$cmd id toto path $T/a/x public path $T/a public install uninstall
$cmd id toto permission aa permission ab permission ac install uninstall
$cmd id toto plug $T/a/x xx $T/b install uninstall
$cmd id toto plug $T/a xx $T/b/x install uninstall
$cmd id toto plug $T/a xx $T/b install uninstall
$cmd id toto plug $T/a xx $T/b permission urn:redpesk:permission:xx:public:export:plug install uninstall
$cmd id toto plug $T/a xx $T/b permission urn:redpesk:permission:xx:partner:export:plug install uninstall
$cmd id toto plug $T/a xx $T/b permission urn:redpesk:permission:PUBLIC:public:export:plug install uninstall
$cmd id toto plug $T/a xx $T/b permission urn:redpesk:permission:PUBLIC:partner:export:plug install uninstall
$cmd id toto plug $T/a PUBLIC $T/b permission urn:redpesk:permission:PUBLIC:public:export:plug install uninstall
$cmd id toto plug $T/a PUBLIC $T/b permission urn:redpesk:permission:PUBLIC:partner:export:plug install uninstall
$cmd id toto plug $T/a PUBLIC $T/b permission urn:redpesk:permission:PUBLIC:partner:export:plug install
rm -r $T

killdaemons
trap - EXIT

sleep 0.5
mkdir -p $H/coverage
gcovr \
	--html-title "coverage report for sec-lsm-manager" \
	--root $R/src \
	--exclude-directories '.*tests.*' \
	--sort-percentage \
	--html --html-details \
	--output $H/coverage/index.html \
	--exclude '.*/main-sec-lsm-manager-cmd.c' \
	--exclude '.*/main-sec-lsm-managerd-launch.c' \
	--exclude '.*/simulation/smack/.*' \
	--exclude-unreachable-branches \
	--keep \
	--object-directory $R/build/src

gcovr \
	--html-title "coverage symmary report for sec-lsm-manager" \
	--root $R/src \
	--exclude-directories '.*tests.*' \
	--sort-percentage \
	--html \
	--output $H/coverage-summary.html \
	--exclude '.*/main-sec-lsm-manager-cmd.c' \
	--exclude '.*/main-sec-lsm-managerd-launch.c' \
	--exclude '.*/simulation/smack/.*' \
	--exclude-unreachable-branches \
	--keep \
	--object-directory $R/build/src

gcovr \
	--html-title "coverage symmary report for sec-lsm-manager" \
	--root $R/src \
	--exclude-directories '.*tests.*' \
	--sort-percentage \
	--txt \
	--output $H/coverage.txt \
	--exclude '.*/main-sec-lsm-manager-cmd.c' \
	--exclude '.*/main-sec-lsm-managerd-launch.c' \
	--exclude '.*/simulation/smack/.*' \
	--exclude-unreachable-branches \
	--delete \
	--object-directory $R/build/src

cut -c-$(grep TOTAL $H/coverage.txt|wc -c) $H/coverage.txt > $H/coverage-summary.txt
