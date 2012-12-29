--TEST--
Test for bug #625: xdebug_get_headers() resets header list
--INI--
xdebug.default_enable=1
--FILE--
<?php
header('Content-type: text/plain');
var_dump( xdebug_get_headers( ) );
var_dump( xdebug_get_headers( ) );
?>
--EXPECTF--
array(1) {
  [0] =>
  string(24) "Content-type: text/plain"
}
array(1) {
  [0] =>
  string(24) "Content-type: text/plain"
}
