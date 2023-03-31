--TEST--
Test for bug #265: Xdebug's error handler breaks error_get_last()
--INI--
xdebug.mode=develop
xdebug.dump_globals=0
xdebug.trace_format=0
xdebug.show_local_vars=0
xdebug.show_error_trace=1
--FILE--
<?php
register_shutdown_function( 'f' );
function f(){
	    var_dump(error_get_last());
}
$a = $b;
var_dump(error_get_last());
$a = $b['no'];
var_dump(error_get_last());
gabba();
?>
--EXPECTF--
%s: Undefined variable%sb in %sbug00265.php on line 6

Call Stack:
%w%f %w%d   1. {main}() %sbug00265.php:0

%sbug00265.php:7:
array(4) {
  'type' =>
  int(%d)
  'message' =>
  string(21) "Undefined variable%sb"
  'file' =>
  string(%d) "%sbug00265.php"
  'line' =>
  int(6)
}

%s: Undefined variable%sb in %sbug00265.php on line 8

Call Stack:
%w%f %w%d   1. {main}() %sbug00265.php:0


%s: Trying to access array offset on %Snull in %sbug00265.php on line 8

Call Stack:
%w%f %w%d   1. {main}() %sbug00265.php:0

%sbug00265.php:9:
array(4) {
  'type' =>
  int(%d)
  'message' =>
  string(%d) "Trying to access array offset on %Snull"
  'file' =>
  string(%d) "%sbug00265.php"
  'line' =>
  int(8)
}


Error: Call to undefined function gabba() in %sbug00265.php on line 10

Call Stack:
%w%f %w%d   1. {main}() %sbug00265.php:0


Fatal error: Uncaught Error: Call to undefined function gabba() in %sbug00265.php on line 10

Error: Call to undefined function gabba() in %sbug00265.php on line 10

Call Stack:
%w%f %w%d   1. {main}() %sbug00265.php:0

%sbug00265.php:4:
array(4) {
  'type' =>
  int(1)
  'message' =>
  string(%d) "Uncaught Error: Call to undefined function gabba() in %sbug00265.php:10
Stack trace:
#0 {main}
  thrown"
  'file' =>
  string(%d) "%sbug00265.php"
  'line' =>
  int(10)
}
