--TEST--
Test with auto-trace
--INI--
xdebug.enable=1
xdebug.auto_trace=1
xdebug.collect_params=1
xdebug.auto_profile=0
--FILE--
<?php
	function foo() {
		echo "bar\n";
	}

	foo();	
	xdebug_dump_function_trace();

	xdebug_start_trace();
	foo();	
	xdebug_dump_function_trace();
	xdebug_stop_trace();

	xdebug_start_trace();
	foo();	
	xdebug_dump_function_trace();
	xdebug_stop_trace();

	foo();	
	xdebug_dump_function_trace();
	xdebug_stop_trace();
?>
--EXPECTF--
bar

Function trace:
    %f      %d   -> {main}() /%s/auto_trace.php:0
    %f      %d     -> foo() /%s/auto_trace.php:6

Notice: Function trace already started in /%s/auto_trace.php on line 9

Call Stack:
    %f      %d   1. {main}() /%s/auto_trace.php:0
    %f      %d   2. xdebug_start_trace() /%s/auto_trace.php:9
bar

Function trace:
    %f      %d   -> {main}() /%s/auto_trace.php:0
    %f      %d     -> foo() /%s/auto_trace.php:6
    %f      %d     -> xdebug_dump_function_trace() /%s/auto_trace.php:7
    %f      %d     -> xdebug_start_trace() /%s/auto_trace.php:9
    %f      %d     -> foo() /%s/auto_trace.php:10
bar

Function trace:
    %f      %d     -> foo() /%s/auto_trace.php:15
bar

Notice: Function tracing was not started, use xdebug_start_trace() before calling this function in /%s/auto_trace.php on line 20

Call Stack:
    %f      %d   1. {main}() /%s/auto_trace.php:0
    %f      %d   2. xdebug_dump_function_trace() /%s/auto_trace.php:20

Notice: Function trace was not started in /%s/auto_trace.php on line 21

Call Stack:
    %f      %d   1. {main}() /%s/auto_trace.php:0
    %f      %d   2. xdebug_stop_trace() /%s/auto_trace.php:21
