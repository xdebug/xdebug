<?php

class A
{
    protected static $_instance;

    protected $_a1;
    protected $_a2;

    public function __construct()
    {
    }
}

echo "works not";
$a = new A();

// Set Breakpoint on sleep and inspect $a
sleep(1);
