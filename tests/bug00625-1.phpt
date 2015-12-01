--TEST--
Test for bug #625: xdebug_get_headers() resets header list (1)
--ENV--
XDEBUG_CONFIG=
--INI--
xdebug.default_enable=1
xdebug.overload_var_dump=2
--FILE--
<?php
header( 'Location: bar');
header( 'HTTP/1.0 404 Not Found' );
var_dump( xdebug_get_headers( ) );
?>
--EXPECTF--
%sbug00625-1.php:4:
array(1) {
  [0] =>
  string(13) "Location: bar"
}
