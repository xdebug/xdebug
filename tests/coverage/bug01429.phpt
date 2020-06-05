--TEST--
Test for bug #1429: Code coverage does not cover null coalesce
--INI--
xdebug.mode=coverage
--FILE--
<?php
xdebug_start_code_coverage();

$c = [
	$a ?? null,
	$b ?? null
];

$cc = xdebug_get_code_coverage()[__FILE__];

echo "line  5 is hit: ", $cc[5] == 1 ? 'yes' : 'no', "\n";
echo "line  6 is hit: ", $cc[6] == 1 ? 'yes' : 'no', "\n";
?>
--EXPECT--
line  5 is hit: yes
line  6 is hit: yes
