<?php
$a = new A;

class A {


    function __construct() {
        $this->a = B::foo();
    }
}

class B {
    private $b = 1;

    static public function foo() {
        return "I'm alive!";
    }
}
?>
