--TEST--
Test for bug #1656: discover_client_host alters header if multiple values are present
--ENV--
I_LIKE_COOKIES=127.0.0.1, 127.0.0.2
--INI--
xdebug.mode=debug
xdebug.start_with_request=yes
xdebug.client_discovery_header=I_LIKE_COOKIES
xdebug.discover_client_host=1
xdebug.client_port=9999
--FILE--
<?php
var_dump( $_SERVER['I_LIKE_COOKIES'] );
?>
--EXPECTF--
Xdebug: [Step Debug] %sTried: 127.0.0.1:9999 (from I_LIKE_COOKIES HTTP header), localhost:9999 (fallback through xdebug.client_host/xdebug.client_port) :-(
string(20) "127.0.0.1, 127.0.0.2"
