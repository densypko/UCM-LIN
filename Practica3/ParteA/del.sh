#!/bin/bash

function delFirst() {
	for (( i=0 ; $i<60; i++ )); do
		sleep 1
		echo remove $i > /proc/modlist
		echo "del $i [ok]"
	done
}
echo "waiting..."
sleep 5
delFirst
exit 0
