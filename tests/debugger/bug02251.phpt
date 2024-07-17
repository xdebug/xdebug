--TEST--
Test for bug #2251: xdebug.log setting not picked up from XDEBUG_CONFIG
--ENV--
XDEBUG_CONFIG=log={TMP}/{RUNID}{TEST_PHP_WORKER}bug02251.log
--INI--
xdebug.mode=debug,develop
default_charset=utf-8
xdebug.filename_format=
xdebug.client_port=9172
xdebug.start_with_request=yes
xdebug.log=
xdebug.log_level=10
--FILE--
<?php
echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'bug02251.log' );
@unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'bug02251.log' );
?>
--EXPECTF--
%A[Step Debug] %sTried: localhost:9172 (through xdebug.client_host/xdebug.client_port).
