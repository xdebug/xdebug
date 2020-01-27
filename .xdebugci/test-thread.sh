#!/bin/bash

TID="$1"
shift

PHP_DIR=${PHP_DIR:-/usr/local/php}

rm -rf /tmp/ptester/thread/${TID}
mkdir -p /tmp/ptester/thread/${TID}
CWD=`pwd`
MYFILE=`realpath $0`
MYDIR=`dirname ${MYFILE}`
cp run-xdebug-tests.php /tmp/ptester/thread/${TID}

cd /tmp/ptester/thread/${TID}

for i in $@; do
	PATH=${PHP_DIR}/$i/bin:$PATH
	
	mkdir -p /tmp/ptester/thread/${TID}/$i/tmp-xdebug
	cp -rT ${CWD} /tmp/ptester/thread/${TID}/$i/tmp-xdebug
	cd /tmp/ptester/thread/${TID}/$i/tmp-xdebug

	sleep 3

	printf "%2d %6d: Rebuilding for %s\n" $TID $BASHPID $i
	./rebuild.sh >/tmp/ptester/logs/$i.build.log 2>&1

	if [ "$?" != "0" ]; then
		printf "%2d %6d: Build failed for %s\n" $TID $BASHPID $i
		echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?><testsuites buildFailed=\"1\" buildLogFile=\"/tmp/ptester/logs/$i.build.log\"/>" >> /tmp/ptester/junit/${i}.xml
	else
		printf "%2d %6d: Testing for %s\n" $TID $BASHPID $i
		UNIQ_RUN_ID="run-$i-id" SKIP_DBGP_TESTS=1 SKIP_UNPARALLEL_TESTS=1 TEST_PHP_EXECUTABLE=`which php` TEST_PHP_JUNIT="/tmp/ptester/junit/$i.xml" php run-xdebug-tests.php /tmp/ptester/thread/${TID}/${i}/tmp-xdebug/tests >/tmp/ptester/logs/$i.log 2>&1
	fi

	/usr/local/php/7.3.6/bin/php -dextension=mongodb.so ${MYDIR}/ingest.php $i
done
