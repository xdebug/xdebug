<?php
function breakOnMe()
{
	echo "break in the breakOnMe function\n";
}

breakOnMe();

$array = range( 0, 7 );
array_reverse( $array );

$array = array_push( $array, 42 );
array_reverse( $array );
?>
