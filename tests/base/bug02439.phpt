--TEST--
Test for bug #2439: Overloaded set_time_limit() always returns false, even when the limit was applied
--INI--
xdebug.mode=develop
--FILE--
<?php
ini_set('max_execution_time', '42');

var_dump(set_time_limit(0));
var_dump(ini_get('max_execution_time'));
?>
--EXPECTF--
%sbug02439.php:%d:
bool(true)
%sbug02439.php:%d:
string(1) "0"
