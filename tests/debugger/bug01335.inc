<?php

namespace TestA\TestB\TestC;

class A
{
    private $data1;

    public function setData1($data)
    {
        $this->data1 = $data;
    }

}

class B extends A
{
    private $data2;

    public function __construct()
    {
        $this->setData1(new C());
        $this->data2 = new C();
    }
}

class C
{
    private $items;

    public function __construct()
    {
        $this->items = [1,2,3,4];
    }
}

$b = new B();

$x = 1; // Breakpoint me, and inspect $b->*TestA\TestB\TestC\A*data1->items
