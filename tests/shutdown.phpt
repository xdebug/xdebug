--TEST--
Shutdown function
--INI--
xdebug.default_enable=1
xdebug.auto_trace=1
xdebug.collect_params=1
xdebug.profiler_enable=0
--FILE--
<?php
	register_shutdown_function('foo');

	function foo() {
		echo "I'm alive!";
	}
?>
--EXPECTF--
I'm alive!
