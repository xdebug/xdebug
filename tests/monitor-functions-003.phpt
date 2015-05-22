--TEST--
Function Monitor: Simple function name
--FILE--
<?php
xdebug_start_function_monitor( [ 'strlen', 'array_push' ] );
var_dump(xdebug_get_monitored_functions());
echo strlen("yes!"), "\n";
var_dump(xdebug_get_monitored_functions());
echo strlen("yes!"), "\n";
var_dump(xdebug_get_monitored_functions());
xdebug_stop_function_monitor();
?>
===DONE===
--EXPECT--
array(0) {
}
4
array(1) {
  [0]=>
  string(6) "strlen"
}
4
array(2) {
  [0]=>
  string(6) "strlen"
  [1]=>
  string(6) "strlen"
}
===DONE===
