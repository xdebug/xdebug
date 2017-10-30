#!/bin/bash
export CFLAGS="-Wall -Werror -Wextra -Wdeclaration-after-statement -Wmissing-field-initializers -Wshadow -Wno-unused-parameter -ggdb3"
phpize
./configure
make all install
EXTENSIONDIR=`php -r 'echo ini_get("extension_dir");'`
echo "zend_extension=${EXTENSIONDIR}/xdebug.so" >> ~/.phpenv/versions/$(phpenv version-name)/etc/php.ini
echo "zend_extension=${EXTENSIONDIR}/xdebug.so" > /tmp/temp-php-config.ini

if [ "${USE_OPCACHE}" = "0" ]; then
	echo "Removing OPcache"
	cat ~/.phpenv/versions/$(phpenv version-name)/etc/php.ini | grep -v opcache > /tmp/temp-without-opcache.ini
	mv /tmp/temp-without-opcache.ini /home/travis/.phpenv/versions/$(phpenv version-name)/etc/php.ini
else
	echo "Keeping OPcache"
	echo "zend_extension=${EXTENSIONDIR}/opcache.so" >> /tmp/temp-php-config.ini
	echo "opcache.enable=1" >> /tmp/temp-php-config.ini
	echo "opcache.enable_cli=1" >> /tmp/temp-php-config.ini
	echo "opcache.enable=1" >> /home/travis/.phpenv/versions/$(phpenv version-name)/etc/php.ini
	echo "opcache.enable_cli=1" >> /home/travis/.phpenv/versions/$(phpenv version-name)/etc/php.ini
fi
