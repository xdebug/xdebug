--TEST--
Test for bug #2214: Array keys aren't escaped in traces
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=1
xdebug.collect_return=0
xdebug.collect_assignments=0
--FILE--
<?php
require_once 'capture-trace.inc';

require dirname( __FILE__ ) . '/bug02214.inc';

xdebug_stop_trace();
?>
--EXPECTF--
Version: %s
File format: 4
TRACE START [%d-%d-%d %d:%d:%d.%d]
2	%d	1	%f	%d
2	%d	0	%f	%d	dirname	0		%sbug02214-002.php	4	1	'%sbug02214-002.php'
2	%d	1	%f	%d
2	%d	0	%f	%d	require	1	%sbug02214.inc	%sbug02214-002.php	4	0
3	%d	0	%f	%d	func	1		%sbug02214.inc	%d	1	['\n' => '\n', '\r' => '\r', '\r\n' => '\r\n']
3	%d	1	%f	%d
2	%d	1	%f	%d
2	%d	0	%f	%d	xdebug_stop_trace	0		%sbug02214-002.php	6	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
