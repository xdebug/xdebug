--TEST--
Test for bug #1999: Show readonly properties (PHP >= 8.1)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.trace_format=0
--FILE--
<?php
require_once 'capture-trace.inc';


class WithReadOnlyProps
{
	static int $static_int = 1;

	function __construct(
		public string $static_string = "two",
		public readonly string $ro_string = "readonly-default",
	) {}
}

$obj = new WithReadOnlyProps(ro_string: "New Value");


xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w               => $tf = '%s' %s
%w%f %w%d     -> WithReadOnlyProps->__construct($static_string = 'two', $ro_string = 'New Value') %sbug01999.php:15
%w               => $this->static_string = 'two' %sbug01999.php:9
%w               => $this->ro_string = 'New Value' %sbug01999.php:9
%w              => $obj = class WithReadOnlyProps { public string $static_string = 'two'; public readonly string $ro_string = 'New Value' } %sbug01999.php:15
%w%f %w%d     -> xdebug_stop_trace() %sbug01999.php:18
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
