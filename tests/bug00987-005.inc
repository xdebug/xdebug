<?php
$a = new StdClass;
$a->foo = 1;
$a->bar = 2;
$a->baz = array( false, 3.14, "foo" );
class b { static $foo = 73; }; $a->b = new b;
$o = (object) array(
	'key' => 'value', 
	1 => 0, 
	-4 => "foo", 
	3.14 => false, 
	-2 => array( 5, 6, 7, 8 => array( "a", "b", "c" ) ),
	5 => $a,
	);

echo gettype($o), "\n";
?>
