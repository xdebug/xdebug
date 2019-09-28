<?php
use Ds\Vector;

$vector = new Vector();
$vector->push([1]);
$vector->push([2]);
$vector->push([3]);

class Foo
{
    private $val;

    public function __construct($val)
    {
        $this->val = $val;
    }
}
$vector = new Vector();
$vector->push(new Foo(1));
$vector->push(new Foo(2));
$vector->push(new Foo(3));

var_export($vector);
?>
