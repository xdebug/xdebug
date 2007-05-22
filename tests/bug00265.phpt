--TEST--
Test for bug #265: Xdebug's error handler breaks error_get_last().
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if(version_compare(zend_version(), "2.0.0-dev", '<')) echo "skip Zend Engine 2 needed\n"; ?>
--INI--
--FILE--
<?php
register_shutdown_function( 'f' );
function f(){
	    var_dump(error_get_last());
}
$a = $b;
var_dump(error_get_last());
$a = $b['no'];
var_dump(error_get_last());
gabba();
?>
--EXPECTF--
Notice: Undefined variable: b in %sbug00265.php on line 6

Call Stack:
%w%f %w%d   1. {main}() %sbug00265.php:0

array(4) {
  ["type"]=>
  int(8)
  ["message"]=>
  string(21) "Undefined variable: b"
  ["file"]=>
  string(46) "%sbug00265.php"
  ["line"]=>
  int(6)
}

Notice: Undefined variable: b in %sbug00265.php on line 8

Call Stack:
%w%f %w%d   1. {main}() %sbug00265.php:0

array(4) {
  ["type"]=>
  int(8)
  ["message"]=>
  string(21) "Undefined variable: b"
  ["file"]=>
  string(46) "%sbug00265.php"
  ["line"]=>
  int(8)
}

Fatal error: Call to undefined function gabba() in %sbug00265.php on line 10

Call Stack:
%w%f %w%d   1. {main}() %sbug00265.php:0

array(4) {
  ["type"]=>
  int(1)
  ["message"]=>
  string(34) "Call to undefined function gabba()"
  ["file"]=>
  string(46) "%sbug00265.php"
  ["line"]=>
  int(10)
}
