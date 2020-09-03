<?php
function a()
{
	$maybe = 2;

	$a = match($maybe + 1) {
		true => "pi",
		3 => "three",
		default => "def",
	};

	return $a;
}

$result = a();
