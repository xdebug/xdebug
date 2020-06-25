--TEST--
Test for bug #570: undefined symbol: zend_memrchr
--INI--
xdebug.mode=develop
xdebug.dump.GET=
xdebug.dump.SERVER=
--FILE--
<?php
function foo() {
	throw new Exception();
}
foo();
?>
--EXPECTF--
Fatal error: Uncaught%sException%sin %sbug00570.php on line 3

Exception:  in %sbug00570.php on line 3

Call Stack:
%w%f %w%d   1. {main}() %sbug00570.php:0
%w%f %w%d   2. foo() %sbug00570.php:5
