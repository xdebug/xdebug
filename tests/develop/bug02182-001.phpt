--TEST--
Test for bug #2182: Segfault with ArrayObject on stack [1]
--INI--
xdebug.mode=develop
error_reporting=E_ALL & ~E_DEPRECATED
--FILE--
<?php
function z($obj) {
throw new Exception();
}

class Z {
public $prop = 42;
}

$obj = new ArrayObject(new Z());
z($obj);
?>
--EXPECTF--
Fatal error: Uncaught Exception in %sbug02182-001.php on line %d

Exception:  in %sbug02182-001.php on line %d

Call Stack:
%w%f %w%d   1. {main}() %sbug02182-001.php:%d
%w%f %w%d   2. z($obj = class ArrayObject { public $prop = 42 }) %sbug02182-001.php:%d
