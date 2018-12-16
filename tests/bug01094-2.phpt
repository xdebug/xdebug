--TEST--
Test for bug #1094: Segmentation fault when attempting to use branch/path coverage (PHP < 7.3 || (PHP >= 7.3 && !opcache))
--SKIPIF--
<?php
if (version_compare(phpversion(), "7.3", '>=') && extension_loaded('zend opcache') ) {
	echo "skip (PHP < 7.3 || (PHP >= 7.3 && !opcache)) needed\n";
}
?>
--FILE--
<?php

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);
foobarbaz(1);
xdebug_stop_code_coverage();

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
