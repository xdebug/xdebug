--TEST--
Test for bug #1343: Wrong values of numerical keys outside 32bit range
--SKIPIF--
<?php if (PHP_INT_SIZE != 4) { echo "skip Only for 32bit platforms"; } ?>
--INI--
xdebug.default_enable=1
html_errors=0
xdebug.overload_var_dump=2
--FILE--
<?php
$ar = array();
$id = (int)730022509303030;
$ar[$id] = "test";
var_dump($id, $ar);
?>
--EXPECTF--
%sbug01343-32bit.php:5:
int(-1671932682)
%sbug01343-32bit.php:5:
array(1) {
  [-1671932682] =>
  string(4) "test"
}
