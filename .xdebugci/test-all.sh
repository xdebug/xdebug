#!/bin/bash

PATTERN=${1:-'7.3*'}

PHP=`which php`
if [[ "${PHP}" == "" ]]; then
	echo "Can't find a PHP binary on the path"
	exit
fi

PHP_DIR=${PHP_DIR:-/usr/local/php}
SYSTEM_CORES=`nproc`
NPROC=${NPROC:-$SYSTEM_CORES}
MYFILE=`realpath $0`
MYDIR=`dirname ${MYFILE}`
PHPS=`ls -vd ${PHP_DIR}/${PATTERN}`

mkdir -p /tmp/ptester
rm -rf /tmp/ptester/group*.lst
mkdir -p /tmp/ptester/logs
rm -rf /tmp/ptester/logs/*
mkdir -p /tmp/ptester/junit
rm -rf /tmp/ptester/junit/*

c=0
for i in $PHPS; do
	v=`echo $i | sed 's@.*/@@'`

	GroupName=`printf group%03d.lst $c`

	echo -n "$v " >> /tmp/ptester/${GroupName}

	c=`expr $c + 1`
	if [[ $c -eq $NPROC ]]; then
		c=0
	fi
done

MAX=`expr $NPROC - 1`
for i in `seq 0 $MAX`; do
	GroupName=`printf group%03d.lst $i`

	if [ -s /tmp/ptester/$GroupName ]; then
		${MYDIR}/test-thread.sh $i `cat /tmp/ptester/$GroupName` &
	fi
done

wait

echo -n "Storing test results... "

php -dextension=mongodb.so ${MYDIR}/ingest.php

echo "DONE"
