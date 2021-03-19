--TEST--
xdebug_mode: Checks return values when off 
--INI--
xdebug.mode=off
--FILE--
<?php
var_dump(xdebug_mode());
var_dump(xdebug_mode('off'));
var_dump(xdebug_mode('develop'));
var_dump(xdebug_mode('coverage'));
var_dump(xdebug_mode('debug'));
var_dump(xdebug_mode('gcstats'));
var_dump(xdebug_mode('profile'));
var_dump(xdebug_mode('trace'));
?>
--EXPECTF--
string(3) "off"
bool(true)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
