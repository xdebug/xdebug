--TEST--
Test for bug #1901: Stack traces are shown (with a broken time) when Xdebug's mode includes 'debug' but not 'develop' or 'trace'
--INI--
xdebug.mode=debug
xdebug.start_with_request=no
--FILE--
<?php
function f() {
	usleep(50000);
	trigger_error("test");
}

f();
?>
--EXPECTF--
Notice: test in %sbug01901.php on line 4
