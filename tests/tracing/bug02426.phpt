--TEST--
Test for bug #2426: xdebug_get_tracefile_name incorrectly throws notice
--INI--
xdebug.mode=develop
html_errors=0
--FILE--
<?php
$tf = xdebug_get_tracefile_name();

var_dump($tf);
?>
--EXPECTF--
%sbug02426.php:%d:
bool(false)
