<?php
function scoped()
{
	$func = function()
	{
		$a = array( // line #1
			1 => "first", // line #2
			2 => "second",
		);
	};

	$func();
}

scoped();
scoped();
?>
