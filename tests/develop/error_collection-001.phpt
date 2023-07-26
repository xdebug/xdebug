--TEST--
Test for collection errors (1)
--INI--
display_errors=1
xdebug.mode=develop
html_errors=0
--FILE--
<?php
xdebug_start_error_collection();

trigger_error("An error", E_USER_WARNING);

echo xdebug_get_collected_errors()[0];
?>
--EXPECTF--
Warning: An error in %serror_collection-001.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %serror_collection-001.php:0
%w%f %w%d   2. trigger_error($message = 'An error', $error_%s = 512) %A
