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

	foreach ( $array as $nr )
	{
		call_user_func_array( 'nested', array( $nr ) );
	}
}

call_user_func_array( 'nested2', array( 7 ) );
foo();
call_user_func( 'nested', 8 );
?>
