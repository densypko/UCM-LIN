#!/bin/bash

function catS() {
	for (( i=0 ; $i<60; i++ )); do
		cat /proc/modlist
		echo "--------------------------------------"
		sleep 1
	done
}
echo "waiting..."
sleep 5
catS
exit 0
