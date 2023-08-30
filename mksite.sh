#!/bin/sh
export PATH=~/redpesk/iotbzh/iotbzh-engineering-process/scripts:$PATH
mk-site.sh $* QA -T "redpesk-core/sec-lsm-manager main index" -S index-order
