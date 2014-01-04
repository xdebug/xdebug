--TEST--
Test for xdebug.halt_level [2]
--INI--
error_level=-1
xdebug.halt_level=0
xdebug.default_enable=1
xdebug.collect_params=0
--FILE--
<?php
ini_set('xdebug.halt_level', E_NOTICE);
trigger_error("Testing");
echo "Hi!\n";

ini_set('xdebug.halt_level', E_USER_NOTICE | E_NOTICE);
trigger_error("Testing");
echo "Hi!\n";

?>
--EXPECTF--
Notice: Testing in %sbug01004-2.php on line 3

Call Stack:
%w%f %w%d   1. {main}() %sbug01004-2.php:0
%w%f %w%d   2. trigger_error() %sbug01004-2.php:3

Hi!

Notice: Testing in %sbug01004-2.php on line 7

Call Stack:
%w%f %w%d   1. {main}() %sbug01004-2.php:0
%w%f %w%d   2. trigger_error() %sbug01004-2.php:7
