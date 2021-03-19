--TEST--
xdebug_mode: Checks return values when enabled
--INI--
xdebug.mode=develop,coverage,debug,gcstats,profile,trace
--FILE--
<?php
var_dump(xdebug_mode());
var_dump(xdebug_mode('off'));
var_dump(xdebug_mode('debug'));
var_dump(xdebug_mode('develop,coverage,debug,gcstats,profile,trace'));
?>
--EXPECTF--
%sxdebug_mode_function-002.php:%d:
string(44) "develop,coverage,debug,gcstats,profile,trace"
%sxdebug_mode_function-002.php:%d:
bool(false)
%sxdebug_mode_function-002.php:%d:
bool(true)
%sxdebug_mode_function-002.php:%d:
bool(true)
