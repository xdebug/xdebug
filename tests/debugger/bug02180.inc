<?php

class Foo extends SplFixedArray {}

$foo = new Foo();
$foo->setSize(7);
var_dump($foo);
$foo[3] = 'three';

echo 123;
