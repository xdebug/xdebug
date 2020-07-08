--TEST--
Test for crash with a destructor
--INI--
xdebug.mode=develop
xdebug.auto_profile=0
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
