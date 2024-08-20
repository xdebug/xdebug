#!/bin/bash

PATTERN=${1:-'^master|^8.*dev'}

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
PHPS=$(ls -v ${PHP_DIR} | egrep ${PATTERN})
USERID=`id -u`
TMP_DIR=/tmp/ptester-${USERID}

mkdir -p ${TMP_DIR}
rm -rf ${TMP_DIR}/group*.lst
mkdir -p ${TMP_DIR}/logs
rm -rf ${TMP_DIR}/logs/*
mkdir -p ${TMP_DIR}/junit
rm -rf ${TMP_DIR}/junit/*

# Storing Run ID
date +'%Y-%m-%d-%H-%M-%S' > ${TMP_DIR}/run-id.txt

c=0
for i in $PHPS; do
	v=`echo $i | sed 's@.*/@@'`

	GroupName=`printf group%03d.lst $c`

	echo -n "$v " >> ${TMP_DIR}/${GroupName}

	c=`expr $c + 1`
	if [[ $c -eq $NPROC ]]; then
		c=0
	fi
done

MAX=`expr $NPROC - 1`
for i in `seq 0 $MAX`; do
	GroupName=`printf group%03d.lst $i`

	if [ -s ${TMP_DIR}/$GroupName ]; then
		PHP_DIR=${PHP_DIR} ${MYDIR}/test-thread.sh $i `cat ${TMP_DIR}/$GroupName` &
	fi
done

wait

echo -n "Storing test results... "

php -dextension=mongodb.so ${MYDIR}/ingest.php

echo "DONE"
