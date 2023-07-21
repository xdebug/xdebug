--TEST--
Test for xdebug.force_error_reporting (display_errors) [3]
--INI--
xdebug.mode=develop
display_errors=1
log_errors=0
xdebug.force_error_reporting=E_ALL
--FILE--
<?php
ini_set('error_reporting', 0);

trigger_error('six', E_USER_WARNING);
trigger_error('seven');
hex2bin('4');
?>
--EXPECTF--
Warning: six in %sforce_error_reporting-display_errors-003.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sforce_error_reporting-display_errors-003.php:0
%w%f %w%d   2. trigger_error($message = 'six', $error_%s = 512) %sforce_error_reporting-display_errors-003.php:4


Notice: seven in %sforce_error_reporting-display_errors-003.php on line 5

Call Stack:
%w%f %w%d   1. {main}() %sforce_error_reporting-display_errors-003.php:0
%w%f %w%d   2. trigger_error($message = 'seven') %sforce_error_reporting-display_errors-003.php:5


Warning: %s in %sforce_error_reporting-display_errors-003.php on line 6

Call Stack:
%w%f %w%d   1. {main}() %sforce_error_reporting-display_errors-003.php:0
%w%f %w%d   2. hex2bin($%s = '4') %sforce_error_reporting-display_errors-003.php:6
