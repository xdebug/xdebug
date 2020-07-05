--TEST--
Test for bug #265: Xdebug's error handler breaks error_get_last() (< PHP 7.4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.4');
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
Notice: Undefined variable: b in %sbug00265-php72.php on line 6

Call Stack:
%w%f %w%d   1. {main}() %sbug00265-php72.php:0

%sbug00265-php72.php:7:
array(4) {
  'type' =>
  int(8)
  'message' =>
  string(21) "Undefined variable: b"
  'file' =>
  string(%d) "%sbug00265-php72.php"
  'line' =>
  int(6)
}

Notice: Undefined variable: b in %sbug00265-php72.php on line 8

Call Stack:
%w%f %w%d   1. {main}() %sbug00265-php72.php:0

%sbug00265-php72.php:9:
array(4) {
  'type' =>
  int(8)
  'message' =>
  string(21) "Undefined variable: b"
  'file' =>
  string(%d) "%sbug00265-php72.php"
  'line' =>
  int(8)
}


Error: Call to undefined function gabba() in %sbug00265-php72.php on line 10

Call Stack:
%w%f %w%d   1. {main}() %sbug00265-php72.php:0


Fatal error: Uncaught Error: Call to undefined function gabba() in %sbug00265-php72.php on line 10

Error: Call to undefined function gabba() in %sbug00265-php72.php on line 10

Call Stack:
%w%f %w%d   1. {main}() %sbug00265-php72.php:0

%sbug00265-php72.php:4:
array(4) {
  'type' =>
  int(1)
  'message' =>
  string(%d) "Uncaught Error: Call to undefined function gabba() in %sbug00265-php72.php:10
Stack trace:
#0 {main}
  thrown"
  'file' =>
  string(%d) "%sbug00265-php72.php"
  'line' =>
  int(10)
}
