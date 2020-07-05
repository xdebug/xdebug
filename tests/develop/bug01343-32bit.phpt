--TEST--
Test for bug #1343: Wrong values of numerical keys outside 32bit range
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('32bit');
?>
--INI--
xdebug.mode=develop
html_errors=0
xdebug.filename_format=
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
