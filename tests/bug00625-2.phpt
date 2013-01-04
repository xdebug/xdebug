--TEST--
Test for bug #625: xdebug_get_headers() resets header list (2)
--INI--
xdebug.default_enable=1
default_charset=utf-8
--FILE--
<?php
header('Content-type: text/plain');
var_dump( xdebug_get_headers( ) );
var_dump( xdebug_get_headers( ) );
?>
--EXPECTF--
array(1) {
  [0] =>
  string(38) "Content-type: text/plain;charset=utf-8"
}
array(1) {
  [0] =>
  string(38) "Content-type: text/plain;charset=utf-8"
}
