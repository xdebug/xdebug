<?php
function func1()
{
	$clazz = 'foo';
	throw new \Exception(
		"The specified platform for '{$clazz}'" .
		" must be a fetchable service or fully qualified class"
	);
}

function func2()
{
	$clazz = 'foo';
	throw new \Exception(
		'The specified platform for \'' . $clazz .
		'\' must be a fetchable service or fully qualified class'
	);
}

function func3()
{
	$clazz = 'foo';
	throw new \Exception(
<<<ENDDOC
		The specified platform for '{$clazz}'
		 must be a fetchable service or fully qualified class
		The specified platform for '{$clazz}'
		 must be a fetchable service or fully qualified class
ENDDOC
	);
}

return 'foo';
?>
