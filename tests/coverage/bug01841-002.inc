<?php
function a()
{
	$maybe = 1;

	$a = match($maybe) {
		1 => "one",
		2 => "twee",
		default => "three",
	};

	return $a;
}

$result = a();
