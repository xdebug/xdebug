--TEST--
Test for circular references
--INI--
xdebug.enable=1
xdebug.auto_trace=0
--FILE--
<?php
	xdebug_start_trace();

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

	xdebug_dump_function_trace();
?>
--EXPECTF--
Function trace:
    %f      %d     -> foo->foo() /%s/phpt.%x:16
    %f      %d     -> bar(class foo {var $a = class foo {var $a = class foo {var $a = ...; var $b = ...}; var $b = class foo {var $a = ...; var $b = ...}}; var $b = class foo {var $a = class foo {var $a = ...; var $b = ...}; var $b = class foo {var $a = ...; var $b = ...}}}) /%s/phpt.%x:17
    %f      %d     -> bar(class foo {var $a = class foo {var $a = class foo {var $a = ...; var $b = ...}; var $b = class foo {var $a = ...; var $b = ...}}; var $b = class foo {var $a = class foo {var $a = ...; var $b = ...}; var $b = class foo {var $a = ...; var $b = ...}}}) /%s/phpt.%x:18
