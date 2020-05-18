--TEST--
Test for bug #1439: Code coverage does not cover is_object (in elseif)
--INI--
xdebug.mode=coverage
--FILE--
<?php

xdebug_start_code_coverage();

$value = new stdClass;

if (!$value) {
	echo "no!\n";
} elseif (is_object($value)) {
	echo "Is object!\n";
}

$cc = xdebug_get_code_coverage()[__FILE__];

echo "line  9 is hit: ", $cc[9] == 1 ? 'yes' : 'no', "\n";
?>
--EXPECT--
Is object!
line  9 is hit: yes
