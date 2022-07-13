--TEST--
Test for bug #535: Code coverage and return before function|class ending
--INI--
xdebug.mode=coverage
--FILE--
<?php
xdebug_start_code_coverage();

class test
{
    function a() {
        return true;
    }
}

$a = new test();
$a->a();

var_dump(xdebug_get_code_coverage());
?>
--EXPECTF--
array(1) {
  ["%sbug00535.php"]=>
  array(4) {
    [7]=>
    int(1)
    [11]=>
    int(1)
    [12]=>
    int(1)
    [14]=>
    int(1)
  }
}

