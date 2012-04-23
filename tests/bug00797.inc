<?php
$a = new A;

class A {
//    private $a;

    function __construct() {
        $this->a = B::foo();
    }
}

class B {
    private $b;

    static public function foo() {
        return "I'm alive!";
    }
}
?>
