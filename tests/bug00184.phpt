--TEST--
Test for bug #184: problem with control chars in code traces
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
	str_replace(array("\r", "\n"), ' ', 'foobar');
	echo file_get_contents($trace_file);
	unlink($trace_file);
	echo "DONE\n";
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d   -> {main}() %sbug00184.php:0
%w%f %w%d     -> xdebug_get_tracefile_name() %sbug00184.php:2
                           >=> '/tmp/trace.%d.xt'
%w%f %w%d     -> str_replace(array (0 => '\r', 1 => '\n'), ' ', 'foobar') %sbug00184.php:3
                           >=> 'foobar'
%w%f %w%d     -> file_get_contents('/tmp/trace.%d.xt') %sbug00184.php:4
DONE
