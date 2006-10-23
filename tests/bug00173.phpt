--TEST--
Test for bug #173: Xdebug segfaults using SPL ArrayIterator.
--INI--
xdebug.default_enable=1
xdebug.auto_trace=1
xdebug.trace_options=0
xdebug.trace_output_dir=/tmp
xdebug.collect_return=1
xdebug.collect_params=1
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
	$trace_file = xdebug_get_tracefile_name();
	new ArrayIterator(NULL);
	echo file_get_contents($trace_file);
	unlink($trace_file);
	echo "DONE\n";
?>
--EXPECTF--
InvalidArgumentException: Passed variable is not an array or object, using empty array instead in %sbug00173.php on line 3

Call Stack:
%w%f%w%d   1. {main}() %sbug00173.php:0
%w%f%w%d   2. ArrayIterator->__construct(null) %sbug00173.php:3


Variables in local scope (#1):
  $trace_file = '%s.%d.xt'
