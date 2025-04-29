--TEST--
Test for bug #1739: Tracing footer not written
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_options=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.dump_globals=0
xdebug.trace_format=0
xdebug.use_compression=0
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

xdebug_start_trace(getTmpFile( '1739'));

function foo() {
	echo "bar\n";
}

foo();
?>
--AFTER--
<?php
require_once __DIR__ . '/../utils.inc';

echo file_get_contents(getTmpFile('1739' ) . '.xt');
unlink(getTmpFile('1739' ) . '.xt');
?>
--EXPECTF--
bar
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> foo() %sbug01739.php:%d
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
