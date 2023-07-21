--TEST--
Test for xdebug.force_error_reporting (display_errors) [2]
--INI--
xdebug.mode=develop
display_errors=1
log_errors=0
xdebug.force_error_reporting=E_USER_WARNING|E_WARNING
--FILE--
<?php
ini_set('error_reporting', 0);

trigger_error('four', E_USER_WARNING);
trigger_error('five');
hex2bin('4');
?>
--EXPECTF--
Warning: four in %sforce_error_reporting-display_errors-002.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sforce_error_reporting-display_errors-002.php:0
%w%f %w%d   2. trigger_error($message = 'four', $error_%s = 512) %sforce_error_reporting-display_errors-002.php:4


Warning: %s in %sforce_error_reporting-display_errors-002.php on line 6

Call Stack:
%w%f %w%d   1. {main}() %sforce_error_reporting-display_errors-002.php:0
%w%f %w%d   2. hex2bin($%s = '4') %sforce_error_reporting-display_errors-002.php:6
