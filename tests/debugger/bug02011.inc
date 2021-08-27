<?php
class WithProtectedClosure
{
	protected $c;

	function __construct() {
		$this->c = function() { echo "Hi!\n"; };
	}
}

$obj = new WithProtectedClosure();

var_dump($obj);
?>
