--TEST--
Test for xdebug.force_display_errors
--INI--
xdebug.mode=develop
display_errors=0
log_errors=0
xdebug.force_display_errors=1
error_reporting=-1
--FILE--
<?php
ini_set('display_errors', 0);

echo "Error:\n";
trigger_error('two', E_USER_NOTICE);
?>
--EXPECTF--
Error:

Notice: two in %sforce_display_errors.php on line 5

Call Stack:
%w%f %w%d   1. {main}() %sforce_display_errors.php:0
%w%f %w%d   2. trigger_error($message = 'two', $error_%s = 1024) %sforce_display_errors.php:5
