--TEST--
Test for bug #1098: Xdebug doesn't make use of __debugInfo
--INI--
xdebug.default_enable=1
xdebug.overload_var_dump=1
--FILE--
<?php
class FOo {
    public $bar = 55;

    function __debugInfo()
    {
        return array( 'foo' => 42 );
    }
}

$f = new Foo();

var_dump($f);
?>
--EXPECT--
class FOo#1 (1) {
  public $foo =>
  int(42)
}
