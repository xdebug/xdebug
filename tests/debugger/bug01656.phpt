--TEST--
Test for bug #1656: remote_connect_back alters header if multiple values are present
--ENV--
I_LIKE_COOKIES=127.0.0.1, 127.0.0.2
XDEBUG_CONFIG="idekey=foobar"
--INI--
xdebug.remote_enable=1
xdebug.remote_addr_header=I_LIKE_COOKIES
xdebug.remote_autostart=1
xdebug.remote_connect_back=1
xdebug.remote_port=9999
--FILE--
<?php
var_dump( $_SERVER['I_LIKE_COOKIES'] );
?>
--EXPECTF--
string(20) "127.0.0.1, 127.0.0.2"
