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
--EXPECT--
array(0) {
}
4
array(1) {
  [0]=>
  array(3) {
    ["function"]=>
    string(6) "strlen"
    ["filename"]=>
    string(67) "/home/derick/dev/php/derickr-xdebug/tests/monitor-functions-003.php"
    ["lineno"]=>
    int(4)
  }
}
4
array(2) {
  [0]=>
  array(3) {
    ["function"]=>
    string(6) "strlen"
    ["filename"]=>
    string(67) "/home/derick/dev/php/derickr-xdebug/tests/monitor-functions-003.php"
    ["lineno"]=>
    int(4)
  }
  [1]=>
  array(3) {
    ["function"]=>
    string(6) "strlen"
    ["filename"]=>
    string(67) "/home/derick/dev/php/derickr-xdebug/tests/monitor-functions-003.php"
    ["lineno"]=>
    int(6)
  }
}
