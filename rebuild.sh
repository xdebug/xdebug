#!/bin/bash

if [[ ! -f "config.m4" ]]; then
	echo "There is no config.m4 file in this directory"
	exit;
fi

if [[ -f "configure.in" ]]; then
	rm configure.in
fi
if [[ -f "configure.ac" ]]; then
	rm configure.ac
fi
if [[ -d "autom4te.cache" ]]; then
	rm -rf autom4te.cache
fi

bit64=`php -n -r 'echo PHP_INT_SIZE == 8 ? "1" : "0";'`
if [[ ${bit64} != "1" ]]; then
	export CFLAGS="-m32"
fi
phpize && ./configure --enable-xdebug-dev && make clean && make all && make install
