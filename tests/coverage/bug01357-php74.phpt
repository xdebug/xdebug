--TEST--
Test for bug #1357: Function signature using variadics is reported as being not executed (>= PHP 7.4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.4');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

class A {
    public function test(...$a) {
        print_r($a);
    }

    public function works($a) {
        echo $a;
    }
}

function works(...$a) {
    print_r($a);
}

(new A)->test('hi');

(new A)->works('hi');

works('hi');

xdebug_stop_code_coverage(false);

var_dump(xdebug_get_code_coverage());
?>
--EXPECTF--
Array
(
    [0] => hi
)
hiArray
(
    [0] => hi
)
array(1) {
  ["%sbug01357-php74.php"]=>
  array(10) {
    [7]=>
    int(1)
    [8]=>
    int(1)
    [11]=>
    int(1)
    [12]=>
    int(1)
    [16]=>
    int(1)
    [17]=>
    int(1)
    [19]=>
    int(1)
    [21]=>
    int(1)
    [23]=>
    int(1)
    [25]=>
    int(1)
  }
}
