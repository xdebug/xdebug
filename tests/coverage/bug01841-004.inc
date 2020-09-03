<?php
function a()
{
	$maybe = 3;

	$a = match(true) {
		$maybe >= 3 => "one",
		$maybe < 3 => "twee",
	};

	return $a;
}

$result = a();
