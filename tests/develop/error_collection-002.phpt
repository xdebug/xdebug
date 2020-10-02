--TEST--
Test for collection errors (2) (fatal error)
--INI--
error_log=NULL
xdebug.mode=develop
--FILE--
<?php
xdebug_start_error_collection();

trigger_error("An error", E_USER_ERROR);

echo "Errors\n";
var_dump( xdebug_get_collected_errors() );
?>
--EXPECTF--
Fatal error: An error in %serror_collection-002.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %serror_collection-002.php:0
%w%f %w%d   2. trigger_error($message = 'An error', $error_%s = 256) %serror_collection-002.php:4
