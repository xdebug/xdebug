<?php
function fe( $a )
{
	foreach($a as $dummy)
	{
		echo "foo\n";
	}
	echo "the end\n";
}

fe( [ 1, 3, 4 ] );
?>
