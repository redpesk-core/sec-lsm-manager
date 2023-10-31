#!/bin/bash

usedpat() {
	grep -oh 'assets/[^)]*' QA/*.md |
	sort -u |
	sed 's:\(.*\.\)[a-zA-Z0-9]*$:-e "^QA/\1[a-zA-Z0-9]*$":'
}

/usr/bin/ls -1 QA/assets/* |
eval grep -v $(usedpat)
