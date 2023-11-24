#!/bin/bash

rootdir=$(realpath $(dirname $0))
builddir=$rootdir/afl-build
afldir=$rootdir/afl
flavour=sma
#flavour=sel

target=slmc-test-simcyn-sim${flavour}
core=${builddir}/src/tests/${target}

mkdir -p $builddir
cd $builddir || exit 1

cmake .. \
	-DCMAKE_C_COMPILER=afl-cc \
	-DWITH_SYSTEMD=ON \
	-DWITH_SMACK=ON \
	-DWITH_SELINUX=ON \
	-DSIMULATE_CYNAGORA=ON \
	-DSIMULATE_SMACK=ON \
	-DSIMULATE_SELINUX=ON \
	-DFORTIFY=ON \
	-DCOMPILE_TEST=ON \
	-DDEBUG=ON

build() {
	local ext=$1
	shift
	[[ $# -gt 0 ]] && export "$@"
	make -B ${target}
	mv src/tests/${target} src/tests/${target}.$ext
}

( build std )
( build asan AFL_USE_ASAN=1 )
#export AFL_USE_MSAN=1
( build ubsan AFL_USE_UBSAN=1 )
#export AFL_USE_CFISAN=1
#export AFL_USE_TSAN=1
#export AFL_USE_LSAN=1

cd $afldir

prog=$builddir/src/tests/${target}

cat << EOC

cd $afldir
afl-fuzz -P explore -M std -i INPUTS -o OUTPUTS -- $prog.std @@
afl-fuzz -P explore -S asan -i INPUTS -o OUTPUTS -- $prog.asan @@
afl-fuzz -P explore -S ubsan -i INPUTS -o OUTPUTS -- $prog.ubsan @@

EOC
