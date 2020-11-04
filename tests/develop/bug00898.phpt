--TEST--
Test for bug #898: Wrong works with empty string key in ArrayObject
--INI--
xdebug.mode=develop
--FILE--
<?php
$example = new ArrayObject;
$example[""] = 'value';

var_dump($example);
?>
--EXPECTF--
%sbug00898.php:5:
class ArrayObject#%d (1) {
  private $storage =>
  array(1) {
    '' =>
    string(5) "value"
  }
}
