#!/bin/sh
cp /etc/httpd/php.ini /etc/httpd/php.ini-normal
cp /etc/httpd/php.ini-xdebug2 /etc/httpd/php.ini
TEST_PHP_EXECUTABLE=`which php-5.2dev` php-5.2dev -dxdebug.auto_trace=0 /home/derick/dev/php/php-5.2dev/run-tests.php tests/*.phpt
cp /etc/httpd/php.ini-normal /etc/httpd/php.ini
