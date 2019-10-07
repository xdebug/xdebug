<?php
abstract class P {

    private $private_prop = ''; // does NOT show up

    protected $protected_prop = ''; // Shows

    public $public_prop = ''; // Shows

    protected function test() {}

}

class C extends P {

    public function __construct() {} // Set "break point" here

}

$c = new C();
?>
