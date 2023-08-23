--TEST--
Test for segmentation fault with xdebug_get_function_stack() and collect_params=1
--INI--
xdebug.mode=develop
xdebug.collect_assignments=0
--FILE--
<?php
function foo($s) {
	print $s;
	var_dump(xdebug_get_function_stack());
}
 
foo('bar');
?>
--EXPECTF--
bar%sbug00022.php:4:
array(2) {
  [0] =>
  array(6) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(6) "{main}"
    'file' =>
    string(%d) "%sbug00022.php"
    'line' =>
    int(0)
    'params' =>
    array(0) {
    }
  }
  [1] =>
  array(6) {
    'time' =>
    double(%f)
    'memory' =>
    int(%d)
    'function' =>
    string(3) "foo"
    'file' =>
    string(%d) "%sbug00022.php"
    'line' =>
    int(7)
    'params' =>
    array(1) {
      's' =>
      string(5) "'bar'"
    }
  }
}
