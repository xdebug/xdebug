--TEST--
Test with auto-trace
--INI--
xdebug.enable=1
xdebug.auto_trace=1
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
    %f      %d   -> {main}() /%s/phpt.%x:0
    %f      %d     -> foo() /%s/phpt.%x:6

Notice: Function trace already started in /%s/phpt.%x on line 9

Call Stack:
    %f      %d   1. {main}() /%s/phpt.%x:0
    %f      %d   2. xdebug_start_trace() /%s/phpt.%x:9
bar

Function trace:
    %f      %d   -> {main}() /%s/phpt.%x:0
    %f      %d     -> foo() /%s/phpt.%x:6
bar

Function trace:
    %f      %d     -> foo() /%s/phpt.%x:15
bar

Notice: Function tracing was not started, use xdebug_start_trace() before calling this function in /%s/phpt.%x on line 20

Call Stack:
    %f      %d   1. {main}() /%s/phpt.%x:0
    %f      %d   2. xdebug_dump_function_trace() /%s/phpt.%x:20

Notice: Function trace was not started in /%s/phpt.%x on line 21

Call Stack:
    %f      %d   1. {main}() /%s/phpt.%x:0
    %f      %d   2. xdebug_stop_trace() /%s/phpt.%x:21
