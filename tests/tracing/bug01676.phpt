--TEST--
Test for bug #1676: xdebug_trace_* deinit and write_footer not called for shutdown functions
--INI--
xdebug.mode=trace
xdebug.trace_format=0
xdebug.start_with_request=yes
xdebug.collect_assignments=0
xdebug.collect_return=0
--FILE--
<?php
function shutdown() {
	global $tf;

	var_dump("in shutdown");

	xdebug_stop_trace();
	if (preg_match('@\.gz$@', $tf)) {
		$fp = gzopen($tf, 'r');
		echo stream_get_contents($fp);
	} else {
		echo file_get_contents($tf);
	}
}


$tf = xdebug_get_tracefile_name();

register_shutdown_function('shutdown');
var_dump("foo");
?>
--EXPECTF--
string(3) "foo"
string(11) "in shutdown"
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d   -> {main}() %sbug01676.php:0
%w%f %w%d     -> xdebug_get_tracefile_name() %sbug01676.php:17
%w%f %w%d     -> register_shutdown_function($callback = 'shutdown') %sbug01676.php:19
%w%f %w%d     -> var_dump($value = 'foo') %sbug01676.php:20
%w%f %w%d   -> shutdown() %sbug01676.php:0
%w%f %w%d     -> var_dump($value = 'in shutdown') %sbug01676.php:5
%w%f %w%d     -> xdebug_stop_trace() %sbug01676.php:7
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
