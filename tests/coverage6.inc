<?php
function ok($a, $b)
{
	if ($a && !$b) {
		echo "A NOT B\n";
	} else if (!$a && $b) {
		echo "B NOT A\n";
	}
}

function loop_test($a)
{
	do {
		echo $a, "\n";
	} while( --$a );
}

ok(true, false);
loop_test(2);
?>
