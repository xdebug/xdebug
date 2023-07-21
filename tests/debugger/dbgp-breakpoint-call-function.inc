<?php
function breakOnMe()
{
	echo "break in the breakOnMe function\n";
}

breakOnMe();

$array = range( 0, 7 );
$array = array_reverse( $array );

array_push( $array, 42 );
$array = array_reverse( $array );
?>
