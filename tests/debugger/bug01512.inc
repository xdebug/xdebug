<?php

namespace TestA\TestB\TestC;

class A
{
    private $items;

    public function __construct()
    {
        $this->items = array(array(1, 2));
    }
}

class B extends A {
    public function foo() {
        $v = array(array("\\\\" => 1));
        echo 1; // Set breakpoint here and inspect $v and $items
    }
}

(new B())->foo();
