#!/bin/bash

for i in /usr/local/php/*; do
	version=`echo $i | sed 's@.*/@@'`
	echo -n "Rebuilding for PHP ${version}: "

	export PATH=/usr/local/php/${version}/bin:/usr/lib/ccache:/usr/local/bin:/usr/bin:/bin

	./rebuild.sh >/tmp/xdebug-for-${version}.log 2>&1

	if [[ $? == 0 ]]; then
		echo "DONE"
	else
		echo "FAIL"
	fi
done
