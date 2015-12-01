--TEST--
xdebug_code_coverage_started()
--INI--
xdebug.default_enable=1
xdebug.extended_info=1
xdebug.coverage_enable=1
xdebug.overload_var_dump=1
--FILE--
<?php
var_dump(xdebug_code_coverage_started());

xdebug_start_code_coverage();
var_dump(xdebug_code_coverage_started());

var_dump(xdebug_get_code_coverage());
var_dump(xdebug_code_coverage_started());

xdebug_stop_code_coverage();
var_dump(xdebug_code_coverage_started());
?>
--EXPECTF--
bool(false)
bool(true)
array(1) {
  '%sxdebug_code_coverage_started.php' =>
  array(2) {
    [5] =>
    int(1)
    [7] =>
    int(1)
  }
}
bool(true)
bool(false)
