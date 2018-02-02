--TEST--
Test for bug #575: Dump super globals contents to error log
--INI--
xdebug.default_enable=1
display_errors=0
log_errors=1
xdebug.force_display_errors=0
xdebug.force_error_reporting=E_ALL
xdebug.collect_params=0
xdebug.dump.SERVER=*
--FILE--
<?php
ini_set('error_reporting', 0);

trigger_error('six', E_USER_WARNING);
trigger_error('seven');
strlen();
?>
--EXPECTF--
PHP Warning:  six in %sbug00575.php on line 4
PHP Stack trace:
PHP   1. {main}() %sbug00575.php:0
PHP   2. trigger_error() %sbug00575.php:4
PHP 
PHP Dump $_SERVER%A
PHP 
PHP Notice:  seven in %sbug00575.php on line 5
PHP Stack trace:
PHP   1. {main}() %sbug00575.php:0
PHP   2. trigger_error() %sbug00575.php:5
PHP 
PHP Dump $_SERVER%A
PHP 
PHP Warning:  %s in %sbug00575.php on line 6
PHP Stack trace:
PHP   1. {main}() %sbug00575.php:0
PHP   2. strlen() %sbug00575.php:6
PHP 
PHP Dump $_SERVER%A
PHP 
