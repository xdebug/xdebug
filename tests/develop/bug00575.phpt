--TEST--
Test for bug #575: Dump super globals contents to error log
--INI--
xdebug.mode=develop
display_errors=0
log_errors=1
error_log=
xdebug.force_display_errors=0
xdebug.force_error_reporting=E_ALL
xdebug.dump.SERVER=*
--FILE--
<?php
ini_set('error_reporting', 0);

trigger_error('six', E_USER_WARNING);
trigger_error('seven');
?>
--EXPECTF--
PHP Warning:  six in %sbug00575.php on line 4
PHP Stack trace:
PHP   1. {main}() %sbug00575.php:0
PHP   2. trigger_error($message = 'six', $error_level = 512) %sbug00575.php:4
PHP%S
PHP Dump $_SERVER%A
PHP%S
PHP Notice:  seven in %sbug00575.php on line 5
PHP Stack trace:
PHP   1. {main}() %sbug00575.php:0
PHP   2. trigger_error($message = 'seven') %sbug00575.php:5
PHP%S
PHP Dump $_SERVER%A
PHP%S
