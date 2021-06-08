--TEST--
Test for bug #1088: Xdebug won't show dead and not executed lines at the 2nd time (!opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!opcache');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
class TestMe
{
    public function test_deadcode()
    {
        $a = 1;
        return;
        $a = 'this line wont be executed';
    }
}

xdebug_start_code_coverage (XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);
$testObj = new TestMe();
$testObj->test_deadcode();
$c = xdebug_get_code_coverage();
ksort($c);
var_dump($c);
xdebug_stop_code_coverage();

xdebug_start_code_coverage (XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);
$testObj = new TestMe();
$testObj->test_deadcode();
$testObj->test_deadcode();
$testObj->test_deadcode();
$c = xdebug_get_code_coverage();
ksort($c);
var_dump($c);
xdebug_stop_code_coverage();
?>
--EXPECTF--
array(1) {
  ["%sbug01088.php"]=>
  array(7) {
    [6]=>
    int(1)
    [7]=>
    int(1)
    [8]=>
    int(-2)
    [9]=>
    int(-2)
    [13]=>
    int(1)
    [14]=>
    int(1)
    [15]=>
    int(1)
  }
}
array(1) {
  ["%sbug01088.php"]=>
  array(9) {
    [6]=>
    int(1)
    [7]=>
    int(1)
    [8]=>
    int(-2)
    [9]=>
    int(-2)
    [21]=>
    int(1)
    [22]=>
    int(1)
    [23]=>
    int(1)
    [24]=>
    int(1)
    [25]=>
    int(1)
  }
}
