<?php

class A
{
    static public $_staticvar;

    protected $var_1;
    protected $var_2;
    protected $var_3;


    public function __construct()
    {
    }
}

class B
{
    function __call($name, $args)
    {
        if ($name == 'testMethod') {
            return new \A();
        }
    }
}

$b = new B();
$x = $b->testMethod();

// Set Breakpoint here
sleep(1);
?>
