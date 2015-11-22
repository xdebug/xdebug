<?php

class BaseClass {
    protected $protected = [1, 2, 3];
    private $private = ['a', 'b', 'c'];
}

class DerivedClass extends BaseClass {
    function __get($name)
    {
        throw new \Exception('Derived class getter called with: ' . $name);
    }
}

try {
    $test = new DerivedClass();
    echo "Statement to break on.\n"; // Breakpoint here
}
catch (\Exception $e) {
    echo $e->getMessage();
}
echo "Statement after try/catch\n";
?>
