<?php
class User
{
	function __construct( private string $name )
	{
	}

	function showName()
	{
		$default = 'default value';

		throw new \Exception( "Don't know the name" );
	}
}

$u = new User( 'Elephpant' );

try
{
	$u->showName();
}
catch ( Exception $e )
{
}

var_dump( $u );
?>
