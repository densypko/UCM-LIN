#!/bin/bash

function addSum() {
	for (( i=0 ; $i<60; i++ )); do
		echo add $i > /proc/modlist
		echo "add $i [ok]"
		sleep 1
	done
	echo "--------------------------------------"
}
echo "waiting..."
sleep 5
echo cleanup > /proc/modlist
addSum
exit 0
