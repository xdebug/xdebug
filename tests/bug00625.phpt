--TEST--
Test for bug #625: xdebug_get_headers() resets header list
--INI--
xdebug.default_enable=1
--FILE--
<?php
header( 'Location: bar');
header( 'HTTP/1.0 404 Not Found' );
var_dump( xdebug_get_headers( ) );
?>
--EXPECTF--
array(1) {
  [0] =>
  string(13) "Location: bar"
}
