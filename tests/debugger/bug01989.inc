<?php

class A
{
    public function __construct()
    {
		xdebug_break();
    }
}

class B extends A
{
    public function __construct()
    {
        parent::__construct();
    }
}

new B();
