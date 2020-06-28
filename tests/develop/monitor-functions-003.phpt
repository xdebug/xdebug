--TEST--
Function Monitor: Simple function name
--INI--
xdebug.mode=develop
--FILE--
<?php
xdebug_start_function_monitor( [ 'strrev', 'array_push' ] );
var_dump(xdebug_get_monitored_functions());
echo strrev("yes!"), "\n";
var_dump(xdebug_get_monitored_functions());
echo strrev("yes!"), "\n";
var_dump(xdebug_get_monitored_functions());
xdebug_stop_function_monitor();
?>
--EXPECTF--
%smonitor-functions-003.php:3:
array(0) {
}
!sey
%smonitor-functions-003.php:5:
array(1) {
  [0] =>
  array(3) {
    'function' =>
    string(6) "strrev"
    'filename' =>
    string(%d) "%smonitor-functions-003.php"
    'lineno' =>
    int(4)
  }
}
!sey
%smonitor-functions-003.php:7:
array(2) {
  [0] =>
  array(3) {
    'function' =>
    string(6) "strrev"
    'filename' =>
    string(%d) "%smonitor-functions-003.php"
    'lineno' =>
    int(4)
  }
  [1] =>
  array(3) {
    'function' =>
    string(6) "strrev"
    'filename' =>
    string(%d) "%smonitor-functions-003.php"
    'lineno' =>
    int(6)
  }
}
