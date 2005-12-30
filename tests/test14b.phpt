--TEST--
Test for circular references (ZE2)
--SKIPIF--
<?php if(version_compare(zend_version(), "2.0.0-dev", '<')) echo "skip Zend Engine 2 needed\n"; ?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
report_memleaks=0
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));

	class foo {

		function foo() {
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
    %f          %d     -> foo->foo() /%s/test14b.php:16
    %f          %d     -> bar(class foo { public $a = ...; public $b = ... }) /%s/test14b.php:17
    %f          %d     -> bar(class foo { public $a = ...; public $b = ... }) /%s/test14b.php:18
    %f          %d     -> file_get_contents('/tmp/%s') /%s/test14b.php:20
