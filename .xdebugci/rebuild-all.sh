#!/bin/bash

FORCE=${1:-"YES"}
PHP_DIR=${2:-"/usr/local/php"}

MYFILE=`realpath $0`
MYDIR=`dirname ${MYFILE}`

if [[ ${FORCE} == "YES" ]]; then
	echo "Forcing builds for all versions"
fi

for i in ${PHP_DIR}/*; do
	version=`echo $i | sed 's@.*/@@'`
	echo -n "Rebuilding for PHP ${version}: "

	FILE=${PHP_DIR}/${version}/lib/php/extensions/*/xdebug.so

	if [ ! -e $FILE ] || [[ ${FORCE} == "YES" ]]; then
		export PATH=${PHP_DIR}/${version}/bin:/usr/lib/ccache:/usr/local/bin:/usr/bin:/bin

		LOG="/tmp/xdebug-for-${version}.log"
		echo -n "Building: "
		${MYDIR}/../rebuild.sh >$LOG 2>&1

		if [[ $? == 0 ]]; then
			echo "DONE"
		else
			echo "FAIL, log is at $LOG"
		fi
	else
		echo "SKIP"
	fi
done
