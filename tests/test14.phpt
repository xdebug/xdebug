--TEST--
Test for circular references (ZE1)
--SKIPIF--
<?php if(version_compare(zend_version(), "2.0.0-dev", '>=')) echo "skip Zend Engine 1 needed\n"; ?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
report_memleaks=0
xdebug.collect_params=1
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.trace_format=0
--FILE--
<?php
	$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));

	class foo {

		function foo() {
			$this->a = &$this;
			$this->b = &$this;
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
    %f      %d     -> foo->foo() /%s/test14.php:16
    %f      %d     -> bar(class foo {var $a = class foo {var $a = class foo {var $a = ...; var $b = ...}; var $b = class foo {var $a = ...; var $b = ...}}; var $b = class foo {var $a = class foo {var $a = ...; var $b = ...}; var $b = class foo {var $a = ...; var $b = ...}}}) /%s/test14.php:17
    %f      %d     -> bar(class foo {var $a = class foo {var $a = class foo {var $a = ...; var $b = ...}; var $b = class foo {var $a = ...; var $b = ...}}; var $b = class foo {var $a = class foo {var $a = ...; var $b = ...}; var $b = class foo {var $a = ...; var $b = ...}}}) /%s/test14.php:18
    %f      %d     -> file_get_contents('/tmp/%s') /%s/test14.php:20
