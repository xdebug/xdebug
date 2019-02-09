#!/bin/bash

TID="$1"
shift

rm -rf /tmp/ptester/thread/${TID}
mkdir -p /tmp/ptester/thread/${TID}
CWD=`pwd`

cp run-xdebug-tests.php /tmp/ptester/thread/${TID}

cd /tmp/ptester/thread/${TID}

for i in $@; do
	echo $TID $BASHPID "Testing for $i"
	PATH=/usr/local/php/$i/bin:$PATH

	mkdir -p /tmp/ptester/thread/${TID}/$i
	cp -r ${CWD}/contrib /tmp/ptester/thread/${TID}/$i
	cp -r ${CWD}/tests /tmp/ptester/thread/${TID}/$i

	SKIP_DBGP_TESTS=1 SKIP_UNPARALLEL_TESTS=1 TEST_PHP_EXECUTABLE=`which php` TEST_PHP_JUNIT="/tmp/ptester/junit/$i.xml" php run-xdebug-tests.php /tmp/ptester/thread/${TID}/${i}/tests >/tmp/ptester/logs/$i.log 2>&1
done
