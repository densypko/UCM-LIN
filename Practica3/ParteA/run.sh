#!/bin/bash

function addSumFast() {
	for (( i=0 ; $i<200; i++ )); do
		echo add $i > /proc/modlist
		echo "--------------------------------------"
		cat /proc/modlist
		sleep 0.1
	done
	echo "--------------------------------------"
}

function addSumMedium() {
	for (( i=250 ; $i<490; i++ )); do
		echo add $i > /proc/modlist
		echo "--------------------------------------"
		cat /proc/modlist
		sleep 0.5
	done
	echo "--------------------------------------"
}

function addSumLow() {
	for (( i=500 ; $i<600; i++ )); do
		echo add $i > /proc/modlist
		echo "--------------------------------------"
		cat /proc/modlist
		sleep 1
	done
	echo "--------------------------------------"
}

function addMulFast() {
	mul=0
	for (( i=0 ; $i<200; i++ )); do
		mul=$i*$i
		echo add $mul > /proc/modlist
		echo "--------------------------------------"
		cat /proc/modlist
		sleep 0.1
	done
	echo "--------------------------------------"
}

function addMulMedium() {
	mul=0
	for (( i=250 ; $i<490; i++ )); do
		mul=$i*$i
		echo add $mul > /proc/modlist
		echo "--------------------------------------"
		cat /proc/modlist
		sleep 0.5
	done
	echo "--------------------------------------"
}

function addMulLow() {
	mul=0
	for (( i=500 ; $i<600; i++ )); do
		mul=$i*$i
		echo add $mul > /proc/modlist
		echo "--------------------------------------"
		cat /proc/modlist
		sleep 1
	done
	echo "--------------------------------------"
}

function delFirst() {
	echo "--------------------------------------"
	for (( i=0 ; $i<200; i++ )); do
		echo remove $i > /proc/modlist
		cat /proc/modlist
		sleep 0.2
		echo "--------------------------------------"
	done
}

function delSecond() {
	echo "--------------------------------------"
	for (( i=500 ; $i<600; i++ )); do
		echo "remove $i" > /proc/modlist
		cat /proc/modlist
		sleep 1
		echo "--------------------------------------"
	done
}

function sorts() {
	echo "--------------------------------------"
	for (( i=0 ; $i<50; i++ )); do
		echo "sort" > /proc/modlist
		cat /proc/modlist
		sleep 10
		echo "--------------------------------------"
	done
}

addSumFast &
addSumMedium &
addSumLow &
addMulFast &
addMulMedium &
addMulLow &
delFirst &
delSecond &
sorts &
exit 0
#echo cleanup > /proc/modlist
