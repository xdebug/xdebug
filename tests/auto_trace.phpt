--TEST--
Test with auto-trace
--INI--
xdebug.default_enable=1
xdebug.auto_trace=1
xdebug.trace_output_dir=/tmp
xdebug.collect_params=1
xdebug.auto_profile=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
--FILE--
<?php
	$trace_file = xdebug_get_tracefile_name();
	function foo() {
		echo "bar\n";
	}

	foo();	
	echo file_get_contents($trace_file);
	unlink($trace_file);
?>
--EXPECTF--
bar

TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d   -> {main}() /%s/auto_trace.php:0
    %f      %d     -> xdebug_get_tracefile_name() /%s/auto_trace.php:2
    %f      %d     -> foo() /%s/auto_trace.php:7
    %f      %d     -> file_get_contents('/tmp/%s') /%s/auto_trace.php:8
