<?php
	function foo2 ($a, $b, $c)
	{
		return foo();
	}

	foo2 (4, array(array('blaat', 5, FALSE)));
?>
