<?php
class MyClass {
	private $a = "b";
	private $b = null;

	public function __construct() {
		$this->b = 'b';
		$c = 'a' . 'b';
	}
	// I hoover in nb over this $a
	public function test($a) {
		echo "a ist: " . $a;
	}
}
  
$a = 1;
$b = 2;
$c = new MyClass();
$f = 3;
$c->test('gogo'); // this call***
$c->test('gogo');
