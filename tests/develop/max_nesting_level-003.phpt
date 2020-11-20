--TEST--
Test for xdebug.max_nesting_level (non develop mode)
--INI--
xdebug.max_stack_frames=-1
xdebug.mode=trace
xdebug.show_local_vars=0
xdebug.max_nesting_level=128
--FILE--
<?php
$a = 0;

function foo()
{
	global $a;

	$a++;

	if ($a >= 500) {
		return;
	}

	foo();
}
foo();

echo $a, "\n";
?>
--EXPECTF--
500
