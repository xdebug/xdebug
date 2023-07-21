--TEST--
Test for bug #625: xdebug_get_headers() resets header list (2)
--ENV--
XDEBUG_CONFIG=
--INI--
xdebug.mode=develop
default_charset=utf-8
xdebug.filename_format=
--FILE--
<?php
header('Content-type: text/plain');
var_dump( xdebug_get_headers( ) );
var_dump( xdebug_get_headers( ) );
?>
--EXPECTF--
%sbug00625-002.php:3:
array(1) {
  [0] =>
  string(38) "Content-type: text/plain;charset=utf-8"
}
%sbug00625-002.php:4:
array(1) {
  [0] =>
  string(38) "Content-type: text/plain;charset=utf-8"
}
