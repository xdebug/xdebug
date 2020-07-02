--TEST--
Test to see if exceptions still work with Xdebug's hook enabled
--INI--
xdebug.mode=develop
xdebug.collect_params=0
--FILE--
<?php

class FooException extends Exception {
}

function a() {
	try {
		throw new FooException('foo');
	} catch (Exception $e) {
		echo "Caught\n";
	}
}

a();
?>
--EXPECT--
Caught
