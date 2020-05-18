--TEST--
Test for bug #1094: Segmentation fault when attempting to use branch/path coverage
--INI--
xdebug.mode=coverage
--FILE--
<?php

function thisWillSegfault()
{
    xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);
}

function foobarbaz($number)
{
    if ($number <= 0) {
        return 'baz';
    }

    if (rand(0, $number) < ($number / 2)) {
        return 'foo';
    } else {
        return 'bar';
    }
}

echo "Trying from within a function...";

thisWillSegfault();
foobarbaz(1);
xdebug_stop_code_coverage();

echo "...done!\n";
?>
--EXPECT--
Trying from within a function......done!
