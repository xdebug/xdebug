<?php
function stackMe($e)
{
	return array_reverse(xdebug_get_function_stack( ['from_exception' => $e ]));
}

function breakOnMe()
{
	throw new Exception("Bite Me");
}

set_exception_handler('stackMe');

$foo = breakOnMe();
?>
