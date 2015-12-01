--TEST--
xdebug_get_function_stack with variadics
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.6", '>=')) echo "skip >= PHP 5.6 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.collect_vars=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
log_errors=1
error_log=
display_errors=0
--FILE--
<?php
function foo( $a, ...$b )
{
	$s = xdebug_get_function_stack();
	var_dump( $s[1] );
}

foo( 42 );
foo( 1, false );
foo( "foo", "bar", 3.1415 );
?>
--EXPECTF--
%sxdebug_get_function_stack_variadic.php:5:
array(4) {
  'function' =>
  string(3) "foo"
  'file' =>
  string(%d) "%sxdebug_get_function_stack_variadic.php"
  'line' =>
  int(8)
  'params' =>
  array(2) {
    'a' =>
    string(2) "42"
    'b' =>
    array(0) {
    }
  }
}
%sxdebug_get_function_stack_variadic.php:5:
array(4) {
  'function' =>
  string(3) "foo"
  'file' =>
  string(%d) "%sxdebug_get_function_stack_variadic.php"
  'line' =>
  int(9)
  'params' =>
  array(2) {
    'a' =>
    string(1) "1"
    'b' =>
    array(1) {
      [1] =>
      string(5) "FALSE"
    }
  }
}
%sxdebug_get_function_stack_variadic.php:5:
array(4) {
  'function' =>
  string(3) "foo"
  'file' =>
  string(%d) "%sxdebug_get_function_stack_variadic.php"
  'line' =>
  int(10)
  'params' =>
  array(2) {
    'a' =>
    string(5) "'foo'"
    'b' =>
    array(2) {
      [1] =>
      string(5) "'bar'"
      [2] =>
      string(6) "3.1415"
    }
  }
}
