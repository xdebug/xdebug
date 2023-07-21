<?php
function loopy($i)
{
	do
	{
		echo $i;

	} while( $i-- );
	echo "\n";
}

function ifthenelse( $a, $b )
{
	if ($a) {
		echo "A HIT\n";
	}
	if ($b) {
		echo "B HIT\n";
	}
}

loopy(0);
ifthenelse( true, false );
ifthenelse( false, true );
?>
