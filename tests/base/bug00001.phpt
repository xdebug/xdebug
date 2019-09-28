--TEST--
Test for crash with a destructor
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
xdebug.profiler_enable=0
--FILE--
<?php
	class A {
		public function __destruct() {
			echo "destructor!\n";
		}
	}

	$obj = new A();

	echo "I'm alive!\n";
?>
--EXPECT--
I'm alive!
destructor!
