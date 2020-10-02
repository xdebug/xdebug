--TEST--
Test for bug #1676: xdebug_trace_* deinit and write_footer not called for shutdown functions (PHP >= 8.0)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=yes
xdebug.collect_assignments=0
xdebug.collect_return=0
--FILE--
<?php
function shutdown() {
	global $tf;

	var_dump("in shutdown");

	xdebug_stop_trace();
	echo file_get_contents($tf);
	unlink($tf);
}

$tf = xdebug_get_tracefile_name();

register_shutdown_function('shutdown');
var_dump("foo");
?>
--EXPECTF--
string(3) "foo"
string(11) "in shutdown"
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d   -> {main}() %sbug01676-php80.php:0
%w%f %w%d     -> xdebug_get_tracefile_name() %sbug01676-php80.php:12
%w%f %w%d     -> register_shutdown_function($callback = 'shutdown') %sbug01676-php80.php:14
%w%f %w%d     -> var_dump($value = 'foo') %sbug01676-php80.php:15
%w%f %w%d   -> shutdown() %sbug01676-php80.php:0
%w%f %w%d     -> var_dump($value = 'in shutdown') %sbug01676-php80.php:5
%w%f %w%d     -> xdebug_stop_trace() %sbug01676-php80.php:7
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
