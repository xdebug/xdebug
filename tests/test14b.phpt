--TEST--
Test for circular references
--INI--
xdebug.enable=1
xdebug.auto_trace=0
report_memleaks=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));

	class foo {

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

	echo file_get_contents($tf);
	unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> foo->__construct() /%s/test14b.php:16
%w%f %w%d     -> bar(class foo { public $a = ...; public $b = ... }) /%s/test14b.php:17
%w%f %w%d     -> bar(class foo { public $a = ...; public $b = ... }) /%s/test14b.php:18
%w%f %w%d     -> file_get_contents('/tmp/%s') /%s/test14b.php:20
