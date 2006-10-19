<?php
function myFunction()
{
	$counter = 1 + 2;
}

register_shutdown_function('myFunction');
?>
