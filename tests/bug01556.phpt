--TEST--
Test for bug #1556: Crash when register_shutdown_function() is called with a function named call_user_func*
--FILE--
<?php
register_shutdown_function('call_user_funca');

function call_user_funca()
{
	echo "in call_user_funca\n";
}

?>
==DONE==
--EXPECTF--
==DONE==
in call_user_funca
