<?php
$v = "";

class Foo
{
    private $a;
    public function __construct($a) {
        $this->a = $a;
    }
}
class Bar extends Foo
{
    public $a;
    public function __construct($fooA, $a) {
        parent::__construct($fooA);
        $this->a = $a;
    }
}

$v = new Foo(new Bar(new Foo(new Foo(2)), 1));

var_dump($v); // breakpoint
