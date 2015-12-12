--TEST--
Test for bug #1140: Tracing with __debugInfo() crashes Xdebug due to a stack overflow
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
xdebug.overload_var_dump=1
--FILE--
<?php
class Foo
{
	function __debugInfo()
	{
		$p = get_object_vars( $this );
		return $p;
	}
}

$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

$a = new Foo;
var_dump( $a );
	
xdebug_stop_trace();
echo file_get_contents( $tf );
unlink( $tf );
?>
== I didn't crash ==
--EXPECTF--
class Foo#1 (0) {
}
TRACE START [%d-%d-%d %d:%d:%d]
    %f     %d     -> var_dump(class Foo {  }) %sbug01140.php:14
    %f     %d     -> xdebug_stop_trace() %sbug01140.php:16
    %f     %d
TRACE END   [%d-%d-%d %d:%d:%d]

== I didn't crash ==
