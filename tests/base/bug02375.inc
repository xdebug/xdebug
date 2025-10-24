<?php

class Example
{
    private int $prop;

    public function getProp(): int
    {
        return $this->prop;
    }
}

$refClass = new ReflectionClass(Example::class);

$object = $refClass->newLazyGhost(function ($object) {
    echo "Initializer called\n";
    $ref = new ReflectionObject($object);
    $p = $ref->getProperty('prop');
    $p->setValue($object, 1);
});

function foo(object $object)
{
	try {
		throw new Exception();
	} catch (Exception $e) {
	}
}

foo($object);

print_r($object);

$object->getProp();

print_r((array) $object); // When Xdebug is loaded, the array is empty.
