<?php
function loopy($i)
{
	for ($j = 0; $j < $i; $j++) {
		echo $j, ' ';
	}
	echo "\n";
}

function ifelse($i)
{
	if ($i == 42) {
		echo "42\n";
	} else {
		echo "!42\n";
		return;
	}
	echo "EOF\n";
}

function trycatch()
{
	try {
		throw new Exception("test");
		echo "Not thrown\n";
	} catch (Exception $e) {
		echo "caught\n";
	}
}

loopy(0);
loopy(4);
ifelse(44);
trycatch();
?>
