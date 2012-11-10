--TEST--
Test for bug #898: Wrong works with empty string key in ArrayObject. (>= PHP 5.3)
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.3", '>=')) echo "skip >= PHP 5.3 needed\n"; ?>
--INI--
xdebug.default_enable=1
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
