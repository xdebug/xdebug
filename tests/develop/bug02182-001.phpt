--TEST--
Test for bug #2182: Segfault with ArrayObject on stack
--INI--
xdebug.mode=develop
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
Fatal error: Uncaught Exception in %sbug02182.php on line %d

Exception:  in %sbug02182.php on line %d

Call Stack:
%w%f %w%d   1. {main}() %sbug02182.php:%d
%w%f %w%d   2. z($obj = class ArrayObject { public $prop = 42 }) %sbug02182.php:%d
