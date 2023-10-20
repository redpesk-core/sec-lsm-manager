#!/bin/bash

DEFS='
	-DVERSION="0"
	-DSIMULATE_CYNAGORA=1
	-DSIMULATE_SELINUX=1
	-DSIMULATE_SMACK=1
	-DWITH_SELINUX=1
	-DWITH_SMACK=1
	-DWITH_SYSTEMD=1
	-D_GNU_SOURCE
'

INCS="
	-I.
	-I./lsm-smack
	-I./lsm-selinux
	-I$HOME/.locenv/sec/include
	-I$HOME/.locenv/socle/include
	-I/usr/include
"

#OPTS="
#	-fdump-analyzer-callgraph
#	-fdump-analyzer-exploded-graph
#"

gcc -fanalyzer $DEFS $INCS $OPTS *.c */*.c */*/*.c



