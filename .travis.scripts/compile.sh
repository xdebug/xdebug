#!/bin/bash
export CFLAGS="-Wall -Wextra -Wdeclaration-after-statement -Wmissing-field-initializers -Wshadow -Wno-unused-parameter -ggdb3"
phpize
./configure
make all install
EXTENSIONDIR=`php -r 'echo ini_get("extension_dir");'`
echo "zend_extension=${EXTENSIONDIR}/xdebug.so" >> ~/.phpenv/versions/$(phpenv version-name)/etc/php.ini
echo "zend_extension=${EXTENSIONDIR}/xdebug.so" > /tmp/temp-php-config.ini

if [[ ${USE_OPCACHE} == "0" ]]; then
	echo "Removing OPcache"
	cat ~/.phpenv/versions/$(phpenv version-name)/etc/php.ini | grep -v opcache > /tmp/temp-without-opcache.ini
	mv /tmp/temp-without-opcache.ini /home/travis/.phpenv/versions/$(phpenv version-name)/etc/php.ini
else
	echo "Keeping OPcache"
fi

echo
echo "temp-php-config.ini"
cat /tmp/temp-php-config.ini

echo
echo "travis.ini"
cat /home/travis/.phpenv/versions/$(phpenv version-name)/etc/conf.d/travis.ini
