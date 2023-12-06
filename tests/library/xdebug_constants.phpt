--TEST--
Test for bug #2223: Xdebug's constants are not available with xdebug.mode=off
--INI--
xdebug.mode=off
--FILE--
<?php
$constants = get_defined_constants(true);

echo "Constants: ", array_key_exists( 'xdebug', $constants ) ? "available: " : "not available\n";
if (array_key_exists('xdebug', $constants)) {
	echo count(array_keys($constants['xdebug'])), "\n";
}
?>
--EXPECT--
Constants: available: 17
