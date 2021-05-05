--TEST--
xdebug_get_function_stack in exception handler
--INI--
xdebug.mode=develop
xdebug.trace_format=0
xdebug.dump_globals=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
log_errors=1
error_log=
display_errors=0
xdebug.filename_format=
--FILE--
<?php
function custom_exception_handler()
{
	var_dump( xdebug_get_function_stack() );
}

function foo( $a )
{
	$g = new stdClass;
	$g->fooBar = 42;

	$pi = M_PI;

	throw new Exception('Simple message');
}

set_exception_handler( 'custom_exception_handler' );

foo( true );

?>
--EXPECTF--
%sxdebug_get_function_stack_local_vars.php:4:
array(1) {
  [0] =>
  array(4) {
    'function' =>
    string(24) "custom_exception_handler"
    'file' =>
    string(%d) "%sxdebug_get_function_stack_local_vars.php"
    'line' =>
    int(0)
    'params' =>
    array(1) {
      [0] =>
      string(%d) "class Exception { %s"...
    }
  }
}
