--TEST--
Test for bug #1782: Make sure we use SameSite=Strict cookies (>= PHP 7.3)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.3');
?>
--ENV--
XDEBUG_CONFIG=idekey=testing
--INI--
xdebug.mode=debug,develop
default_charset=utf-8
xdebug.filename_format=
xdebug.client_port=9172
--FILE--
<?php
var_dump( xdebug_get_headers( ) );
?>
--EXPECTF--
Xdebug: [Step Debug] %sTried: localhost:9172 (through xdebug.client_host/xdebug.client_port) :-(
%sbug01782.php:2:
array(1) {
  [0] =>
  string(%d) "Set-Cookie: XDEBUG_SESSION=testing; expires=%s; Max-Age=3%d; path=/; SameSite=Strict"
}
