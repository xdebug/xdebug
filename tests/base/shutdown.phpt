--TEST--
Shutdown function
--INI--
xdebug.mode=develop
xdebug.collect_params=1
--FILE--
<?php
	register_shutdown_function('foo');

	function foo() {
		echo "I'm alive!";
	}
?>
--EXPECTF--
I'm alive!
