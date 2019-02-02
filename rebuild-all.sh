#!/bin/bash

for i in /usr/local/php/*; do
	version=`echo $i | sed 's@.*/@@'`
	echo -n "Rebuilding for PHP ${version}: "

	export PATH=/usr/local/php/${version}/bin:/usr/lib/ccache:/usr/local/bin:/usr/bin:/bin

	if [[ "${version}" =~ "32bit" ]]; then
		echo -n "(in 32-bit mode) "
		./rebuild-32bit.sh >/tmp/xdebug-for-${version}.log 2>&1
	else
		./rebuild.sh >/tmp/xdebug-for-${version}.log 2>&1
	fi

	echo "DONE"
done
