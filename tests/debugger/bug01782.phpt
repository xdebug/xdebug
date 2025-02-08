--TEST--
Test for bug #1782: Make sure we use SameSite=Lax cookies (>= PHP 7.3)
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
xdebug.log={TMP}/{RUNID}{TEST_PHP_WORKER}bug01782.txt
xdebug.log_level=10
--FILE--
<?php
var_dump( xdebug_get_headers( ) );
echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'bug01782.txt' );
@unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'bug01782.txt' );
?>
--EXPECTF--
%sbug01782.php:2:
array(1) {
  [0] =>
  string(%d) "Set-Cookie: XDEBUG_SESSION=testing; path=/; SameSite=Lax"
}
%A[Step Debug] %sTried: localhost:9172 (through xdebug.client_host/xdebug.client_port).
