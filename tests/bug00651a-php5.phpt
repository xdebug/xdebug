--TEST--
Test for bug #651: Incorrect code coverage after ! empty() in conditional (< PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
--INI--
xdebug.overload_var_dump=0
--FILE--
<?php

xdebug_start_code_coverage(XDEBUG_CC_UNUSED);

function repeat($x)
{
    if ( ! empty($x)
        AND $x !== 1
        AND $x !== 2
        AND $x !== 3)
    {
        $y = 'covered';
    }
}

repeat(0);
repeat(1);
repeat(2);
repeat(3);
repeat(4);

var_dump(xdebug_get_code_coverage());
?>
--EXPECTF--
array(1) {
  ["%sbug00651a-php5.php"]=>
  array(15) {
    [5]=>
    int(1)
    [7]=>
    int(1)
    [8]=>
    int(1)
    [9]=>
    int(1)
    [10]=>
    int(1)
    [11]=>
    int(1)
    [12]=>
    int(1)
    [13]=>
    int(1)
    [14]=>
    int(1)
    [16]=>
    int(1)
    [17]=>
    int(1)
    [18]=>
    int(1)
    [19]=>
    int(1)
    [20]=>
    int(1)
    [22]=>
    int(1)
  }
}
