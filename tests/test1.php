<?php
	include ('test_class.php');

	function foo ($a)
	{
		$c = new een();
		$b = $a * 3;
		$c->foo2 ($b, array ('blaat', 5, FALSE));
		return $b;
	}

	echo foo (5);

?>
