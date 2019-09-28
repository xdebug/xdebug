<?php
function scoped()
{
	eval('
	$a = array( // line #1
		1 => "first", // line #2
		2 => "second",
	);
	$b = array( // line #1
		1 => "first", // line #2
		2 => "second",
	);
	');
}

scoped();
?>
