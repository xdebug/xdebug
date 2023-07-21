<?php

class A
{
    private $testA;
    private $testC;
    
    public function __construct()
    {
        $this->testA = 'testable';
		$this->testC = new B;
    }
}

class B
{
	private $testB;
}

$foo = 23;

$object = new A();
$iterator = new RecursiveArrayIterator($object);
$clone = $iterator->getArrayCopy();

echo $foo;
?>
