--TEST--
Test for bug #1954: Calling xdebug_start_trace without mode including tracing results in a fatal error
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--INI--
xdebug.mode=off
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));
var_dump( $tf );
?>
--EXPECTF--
Notice: Functionality is not enabled in %sbug01954.php on line 2
NULL
