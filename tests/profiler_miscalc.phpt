--TEST--
Test with internal callbacks
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
--FILE--
<?php
xdebug_start_profiling();

function foo() {
	xdebug_dump_function_profile();
	exit;
}

foo();
?>
--EXPECTF--
Execution Time Profile (sorted by line numbers)
-----------------------------------------------------------------------------------
Time Taken    Number of Calls    Function Name    Location
-----------------------------------------------------------------------------------
0.%d    1    *foo()    /%s/profiler_miscalc.php:9
-----------------------------------------------------------------------------------
Opcode Compiling:                             %f
Function Execution:     0.%d
Ambient Code Execution: 0.%d
Total Execution:                              %f
-----------------------------------------------------------------------------------
Total Processing:                             %f
-----------------------------------------------------------------------------------
