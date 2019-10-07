<?php
class A extends ArrayIterator
{
    protected $aa;
    public function __construct()
    {
        $this->aa = 302;
        parent::__construct(array( 2, 3, 4, 5));
    }
}

$a = new A();
var_dump($a); // put breakpoint here
?>
