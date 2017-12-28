<?php
namespace Stack;

class Three
{
	function __construct( $object )
	{
		$this->obj = $object;
	}

	function callObj( string $name, ...$arguments )
	{
		return $this->obj->$name( ...$arguments );
	}

	function error( $value )
	{
		trigger_error( $value, E_USER_WARNING );
	}
}
