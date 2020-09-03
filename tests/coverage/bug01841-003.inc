<?php
function a()
{
	$maybe = 3;

	$a = match($maybe) {
		1 => "one",
		2 => "twee",

	};

	return $a;
}

try {
	$result = a();
} catch (Error) {}
