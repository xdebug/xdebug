--TEST--
xdebug_code_coverage_started()
--INI--
xdebug.mode=coverage
--FILE--
<?php
var_dump(xdebug_code_coverage_started());

xdebug_start_code_coverage();
var_dump(xdebug_code_coverage_started());

xdebug_get_code_coverage();
var_dump(xdebug_code_coverage_started());

xdebug_stop_code_coverage();
var_dump(xdebug_code_coverage_started());
?>
--EXPECTF--
bool(false)
bool(true)
bool(true)
bool(false)
