<?php

function sum($a, $b) {
    return $a + $b;
}

echo error_reporting() . PHP_EOL;
error_reporting(E_ALL);
echo error_reporting() . PHP_EOL;

$a = 1 + 179;
echo $a . PHP_EOL;
