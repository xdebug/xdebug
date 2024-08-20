#!/bin/bash

TID="$1"
shift

PHP_DIR=${PHP_DIR:-/usr/local/php}
USERID=`id -u`
TMP_DIR=/tmp/ptester-${USERID}

rm -rf ${TMP_DIR}/thread/${TID}
mkdir -p ${TMP_DIR}/thread/${TID}
CWD=`pwd`
MYFILE=`realpath $0`
MYDIR=`dirname ${MYFILE}`
cp run-xdebug-tests.php ${TMP_DIR}/thread/${TID}

cd ${TMP_DIR}/thread/${TID}

for i in $@; do
	PATH=${PHP_DIR}/$i/bin:$PATH

	BASEDIR=${TMP_DIR}/thread/${TID}/$i/tmp-xdebug
	mkdir -p ${BASEDIR}
	mkdir -p ${BASEDIR}/tmp
	cp -rT ${CWD} ${BASEDIR}
	cd ${BASEDIR}

	sleep 3

	printf "%2d %6d: Rebuilding for %s\n" $TID $BASHPID $i
	./rebuild.sh >${TMP_DIR}/logs/$i.build.log 2>&1

	if [ "$?" != "0" ]; then
		printf "%2d %6d: Build failed for %s\n" $TID $BASHPID $i
		echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?><testsuites buildFailed=\"1\" buildLogFile=\"${TMP_DIR}/logs/$i.build.log\"/>" >> ${TMP_DIR}/junit/${i}.xml
	else
		printf "%2d %6d: Testing for %s\n" $TID $BASHPID $i
		TEST_TMP_DIR=${BASEDIR}/tmp UNIQ_RUN_ID="run-$i-id" SKIP_UNPARALLEL_TESTS=1 TEST_PHP_EXECUTABLE=`which php` TEST_PHP_JUNIT="${TMP_DIR}/junit/$i.xml" php run-xdebug-tests.php ${TMP_DIR}/thread/${TID}/${i}/tmp-xdebug/tests >${TMP_DIR}/logs/$i.log 2>&1
	fi

	/usr/local/php/8.3.7/bin/php -dextension=mongodb.so ${MYDIR}/ingest.php $i
done
