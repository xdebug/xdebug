--TEST--
Test for bug #1919: Crash with XDEBUG_FILTER_TRACING without xdebug.mode=trace
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!win');
?>
--INI--
xdebug.mode=develop
xdebug.log={TMP}/{RUNID}{TEST_PHP_WORKER}issue1919-002.txt
--FILE--
<?php
xdebug_set_filter(XDEBUG_FILTER_TRACING, XDEBUG_PATH_INCLUDE, []);
echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'issue1919-002.txt' );
@unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'issue1919-002.txt' );
?>
--EXPECTF--
[%d] Log opened at %s
[%d] [Base] WARN: Can not set a filter for tracing, because Xdebug mode does not include 'trace'
