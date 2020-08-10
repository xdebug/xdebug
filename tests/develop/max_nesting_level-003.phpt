--TEST--
Test for xdebug.max_nesting_level (unconfigured) [2]
--FILE--
<?php

function foo(int $i) : int
{
    if ($i >= 10000) {
        return $i;
    }

	return foo($i + 1);
}
echo foo(0);
?>
--EXPECTF--
10000
