<?php
class foo {
	private $bar;

	function __construct()
	{
		$this->bar = 42;
	}

	function __debugInfo()
	{
		return [ 'bar' => 50 ];
	}
}

$f = new foo();
var_dump( $f );
