<?php
function nested($a)
{
	usleep(500);
	return $a * 3;
}

function nested2($a)
{
	usleep(500);
	return $a * M_PI;
}


function foo() {
	$array = array( 1, 2, 3, 4, 5, 6 );

	foreach ( $array as $nr ) {
		$c = 'call_user_func_array';
		$c( 'nested', array( $nr ) );
	}
}
$cufa = 'call_user_func_array'; $cuf = 'call_user_func';
$cufa( 'nested2', array( 7 ) );
foo();
$cuf( 'nested', 8 );
?>
