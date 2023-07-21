--TEST--
Test with include file
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=0
xdebug.var_display_max_depth=1
xdebug.var_display_max_children=3
--FILE--
<?php
require_once 'capture-trace.inc';
function foo ($a)
{
	$c = new een();
	$b = $a * 3;
	$c->foo2 ($b, array ('blaat', 5, FALSE));
	return $b;
}

include ('test_class.inc');

echo foo(5), "\n";
xdebug_stop_trace();
?>
--EXPECTF--
15
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> include(%stest_class.inc) %sinclude-file.php:11
%w%f %w%d     -> foo($a = 5) %sinclude-file.php:13
%w%f %w%d       -> een->foo2(15, [0 => 'blaat', 1 => 5, 2 => FALSE]) %sinclude-file.php:7
%w%f %w%d         -> een->hang() %stest_class.inc:10
%w%f %w%d     -> xdebug_stop_trace() %sinclude-file.php:14
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
