--TEST--
Test for segmentation fault with xdebug_get_function_stack() and collect_params=1. (ZE1)
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if(version_compare(zend_version(), "2.0.0-dev", '>')) echo "skip Zend Engine 1 needed\n"; ?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_assignments=0
xdebug.profiler_enable=0
--FILE--
<?php
function foo($s) {
	print $s;
	var_dump(xdebug_get_function_stack());
}
 
foo('bar');
?>
--EXPECTF--
bararray(2) {
  [0]=>
  array(4) {
    ["function"]=>
    string(6) "{main}"
    ["file"]=>
    string(%d) "/%s/bug00022-ze1.php"
    ["line"]=>
    int(0)
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(4) {
    ["function"]=>
    string(3) "foo"
    ["file"]=>
    string(%d) "/%s/bug00022-ze1.php"
    ["line"]=>
    int(7)
    ["params"]=>
    array(1) {
      [0]=>
      string(5) "'bar'"
    }
  }
}
