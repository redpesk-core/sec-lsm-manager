#!/bin/bash

rootdir=$(realpath $(dirname $0))
builddir=$rootdir/afl-build
afldir=$rootdir/afl

#export AFL_USE_ASAN=1
#export AFL_USE_MSAN=1
#export AFL_USE_UBSAN=1
#export AFL_USE_CFISAN=1
#export AFL_USE_TSAN=1
#export AFL_USE_LSAN=1

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

make -j

cd $afldir

prog=$builddir/src/tests/slmc-test-simcyn-simsma
#prog=$builddir/src/tests/slmc-test-simcyn-simsel

afl-fuzz -i INPUTS -o OUTPUTS -- $prog

