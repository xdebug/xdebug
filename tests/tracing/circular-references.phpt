--TEST--
Test for circular references
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
report_memleaks=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
require_once 'capture-trace.inc';

class foo {
	public $a, $b;
	function __construct() {
		$this->a = $this;
		$this->b = $this;
	}

}

function bar($o) {
}

$f = new foo();
bar($f);
bar($f);

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> foo->__construct() %scircular-references.php:16
%w%f %w%d     -> bar($o = class foo { public $a = ...; public $b = ... }) %scircular-references.php:17
%w%f %w%d     -> bar($o = class foo { public $a = ...; public $b = ... }) %scircular-references.php:18
%w%f %w%d     -> xdebug_stop_trace() %scircular-references.php:20
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
