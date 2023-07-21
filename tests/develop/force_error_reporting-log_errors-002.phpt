--TEST--
Test for xdebug.force_error_reporting (log_errors) [2]
--INI--
xdebug.mode=develop
display_errors=0
log_errors=1
error_log=
xdebug.force_display_errors=0
xdebug.force_error_reporting=E_USER_WARNING|E_WARNING
--FILE--
<?php
ini_set('error_reporting', 0);

trigger_error('four', E_USER_WARNING);
trigger_error('five');
hex2bin('4');
?>
--EXPECTF--
PHP Warning:  four in %sforce_error_reporting-log_errors-002.php on line 4
PHP Stack trace:
PHP   1. {main}() %sforce_error_reporting-log_errors-002.php:0
PHP   2. trigger_error($message = 'four', $error_%s = 512) %sforce_error_reporting-log_errors-002.php:4
PHP Warning:  %s in %sforce_error_reporting-log_errors-002.php on line 6
PHP Stack trace:
PHP   1. {main}() %sforce_error_reporting-log_errors-002.php:0
PHP   2. hex2bin($%s = '4') %sforce_error_reporting-log_errors-002.php:6
