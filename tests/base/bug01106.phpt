--TEST--
Test for bug #1106: A thrown Exception after a class with __debugInfo gives 2 errors
--INI--
xdebug.mode=develop
xdebug.show_local_vars=1
--FILE--
<?php

class A
{
    public function __debugInfo() { return array();}
}

$c = new A();
try{
    throw new \Exception("neee");
}
catch (\Exception $e) {
    die("all fine");
}
?>
--EXPECT--
all fine
