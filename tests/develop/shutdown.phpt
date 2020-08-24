--TEST--
Shutdown function
--INI--
xdebug.mode=develop
--FILE--
<?php
	register_shutdown_function('foo');

	function foo() {
		echo "I'm alive!";
	}
?>
--EXPECTF--
I'm alive!
