<?php
function foo() {
	$array = array( 1, 2, 3, 4, 5, 6 );

	$cb = function($a) { usleep(500); return $a * 3; };

	array_walk( $array, $cb );

	var_dump( $cb(7) );
}

foo();
?>
