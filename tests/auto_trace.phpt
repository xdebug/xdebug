--TEST--
Test with auto-trace
--INI--
xdebug.enable=1
xdebug.auto_trace=1
xdebug.auto_trace_file=/tmp/xdebug_auto_trace.test
xdebug.collect_params=1
xdebug.auto_profile=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
--FILE--
<?php
	function foo() {
		echo "bar\n";
	}

	foo();	
	echo file_get_contents('/tmp/xdebug_auto_trace.test.xt');
	unlink('/tmp/xdebug_auto_trace.test.xt');
?>
--EXPECTF--
bar

TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d   -> {main}() /%s/auto_trace.php:0
    %f      %d     -> foo() /%s/auto_trace.php:6
    %f      %d     -> file_get_contents('/tmp/%s') /%s/auto_trace.php:7
