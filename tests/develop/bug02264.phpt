--TEST--
Test for bug #2264: Rounding of fractional numbers
--INI--
xdebug.mode=off
--FILE--
<?php
$s = 342_500;
$s *= 0.7;

var_dump($s);

xdebug_var_dump($s);
?>
--EXPECTF--
float(239749.99999999997)
%sbug02264.php:%d:
double(239749.99999999997)
