--TEST--
Function Monitor: Simple function name
--INI--
xdebug.mode=develop
--FILE--
<?php
xdebug_start_function_monitor( [ 'strrev', 'array_push' ] );

include __DIR__ . '/monitor-functions-003.inc';

xdebug_stop_function_monitor();
?>
--EXPECTF--
%smonitor-functions-003.inc:2:
array(0) {
}
!sey
%smonitor-functions-003.inc:4:
array(1) {
  [0] =>
  array(3) {
    'function' =>
    string(6) "strrev"
    'filename' =>
    string(%d) "%smonitor-functions-003.inc"
    'lineno' =>
    int(3)
  }
}
!sey
%smonitor-functions-003.inc:6:
array(2) {
  [0] =>
  array(3) {
    'function' =>
    string(6) "strrev"
    'filename' =>
    string(%d) "%smonitor-functions-003.inc"
    'lineno' =>
    int(3)
  }
  [1] =>
  array(3) {
    'function' =>
    string(6) "strrev"
    'filename' =>
    string(%d) "%smonitor-functions-003.inc"
    'lineno' =>
    int(5)
  }
}
