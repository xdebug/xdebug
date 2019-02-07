#!/bin/bash

FORCE=${1:-"YES"}

if [[ ${FORCE} == "YES" ]]; then
	echo "Forcing builds for all versions"
fi

for i in /usr/local/php/*; do
	version=`echo $i | sed 's@.*/@@'`
	echo -n "Rebuilding for PHP ${version}: "

	FILE=/usr/local/php/${version}/lib/php/extensions/*/xdebug.so

	if [ ! -e $FILE ] || [[ ${FORCE} == "YES" ]]; then
		export PATH=/usr/local/php/${version}/bin:/usr/lib/ccache:/usr/local/bin:/usr/bin:/bin

		LOG="/tmp/xdebug-for-${version}.log"
		echo -n "Building: "
		./rebuild.sh >$LOG 2>&1

		if [[ $? == 0 ]]; then
			echo "DONE"
		else
			echo "FAIL, log is at $LOG"
		fi
	else
		echo "SKIP"
	fi
done
