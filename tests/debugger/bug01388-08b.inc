<?php
$a = array( // line #1
    1 => "first", // line #2
    2 => "second",
);

function scoped()
{
	$a = array( // line #1
		1 => "first", // line #2
		2 => "second",
	);

	return $a;
}
?>
