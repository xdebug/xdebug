--TEST--
Test for bug #1394: Code coverage does not cover instanceof (in elseif)
--INI--
xdebug.mode=coverage
--FILE--
<?php

xdebug_start_code_coverage();

function whatAmI($var) {
    if (is_string($var)) {
        return 'string';
    } elseif ($var instanceof \stdClass) {
        return '\stdClass';
    } elseif (is_scalar($var)) {
        return 'scalar';
    }
    return 'dunno!';
}

whatAmI(new \stdClass);
whatAmI(123);
whatAmI('string');
whatAmI(new DateTime());

$cc = xdebug_get_code_coverage()[__FILE__];

echo "line  8 is hit: ", $cc[8] == 1 ? 'yes' : 'no', "\n";
echo "line 10 is hit: ", $cc[8] == 1 ? 'yes' : 'no', "\n";
?>
--EXPECT--
line  8 is hit: yes
line 10 is hit: yes
