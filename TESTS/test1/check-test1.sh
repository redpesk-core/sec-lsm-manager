#!/bin/bash

CDPATH=
cd $(dirname $0)

TAP() {
	cnt=0
	echo "TAP version 14"
	for ref in references/*.reference
	do
		cnt=$(expr $cnt + 1)
		name=$(basename $ref .reference)
		resu=results/$name.result
		if ! cmp --quiet $ref $resu
		then
			echo -n "not "
		fi
		echo "ok $cnt - $name"
	done
	echo "1..$cnt"
}

TAP | tee test1.tap

