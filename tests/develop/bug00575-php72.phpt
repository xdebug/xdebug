--TEST--
Test for bug #575: Dump super globals contents to error log (PHP < 8)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 8');
?>
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
strlen();
?>
--EXPECTF--
PHP Warning:  six in %sbug00575-php72.php on line 4
PHP Stack trace:
PHP   1. {main}() %sbug00575-php72.php:0
PHP   2. trigger_error($message = 'six', $error_type = 512) %sbug00575-php72.php:4
PHP 
PHP Dump $_SERVER%A
PHP 
PHP Notice:  seven in %sbug00575-php72.php on line 5
PHP Stack trace:
PHP   1. {main}() %sbug00575-php72.php:0
PHP   2. trigger_error($message = 'seven') %sbug00575-php72.php:5
PHP 
PHP Dump $_SERVER%A
PHP 
PHP Warning:  %s in %sbug00575-php72.php on line 6
PHP Stack trace:
PHP   1. {main}() %sbug00575-php72.php:0
PHP   2. strlen() %sbug00575-php72.php:6
PHP 
PHP Dump $_SERVER%A
PHP 
