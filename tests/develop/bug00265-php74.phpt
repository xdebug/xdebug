--TEST--
Test for bug #265: Xdebug's error handler breaks error_get_last() (>= PHP 7.4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.4');
?>
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
%s: Undefined variable%sb in %sbug00265-php74.php on line 6

Call Stack:
%w%f %w%d   1. {main}() %sbug00265-php74.php:0

%sbug00265-php74.php:7:
array(4) {
  'type' =>
  int(%d)
  'message' =>
  string(21) "Undefined variable%sb"
  'file' =>
  string(%d) "%sbug00265-php74.php"
  'line' =>
  int(6)
}

%s: Undefined variable%sb in %sbug00265-php74.php on line 8

Call Stack:
%w%f %w%d   1. {main}() %sbug00265-php74.php:0


%s: Trying to access array offset on value of type null in %sbug00265-php74.php on line 8

Call Stack:
%w%f %w%d   1. {main}() %sbug00265-php74.php:0

%sbug00265-php74.php:9:
array(4) {
  'type' =>
  int(%d)
  'message' =>
  string(51) "Trying to access array offset on value of type null"
  'file' =>
  string(%d) "%sbug00265-php74.php"
  'line' =>
  int(8)
}


Error: Call to undefined function gabba() in %sbug00265-php74.php on line 10

Call Stack:
%w%f %w%d   1. {main}() %sbug00265-php74.php:0


Fatal error: Uncaught Error: Call to undefined function gabba() in %sbug00265-php74.php on line 10

Error: Call to undefined function gabba() in %sbug00265-php74.php on line 10

Call Stack:
%w%f %w%d   1. {main}() %sbug00265-php74.php:0

%sbug00265-php74.php:4:
array(4) {
  'type' =>
  int(1)
  'message' =>
  string(%d) "Uncaught Error: Call to undefined function gabba() in %sbug00265-php74.php:10
Stack trace:
#0 {main}
  thrown"
  'file' =>
  string(%d) "%sbug00265-php74.php"
  'line' =>
  int(10)
}
