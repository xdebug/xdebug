<?php
function add($a, $b)
{
    return $a + $b;
}

function f($n)
{
    $sum = 0;
    for ($i = 0; $i < $n; $i++) {
        $sum = add($sum, $i);
    }
    return $sum;
}

f(3);
f(2);
