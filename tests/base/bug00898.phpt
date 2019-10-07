--TEST--
Test for bug #898: Wrong works with empty string key in ArrayObject
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.overload_var_dump=1
--FILE--
<?php
$example = new ArrayObject;
$example[""] = 'value';

var_dump($example);
?>
--EXPECTF--
class ArrayObject#1 (1) {
  private $storage =>
  array(1) {
    '' =>
    string(5) "value"
  }
}
