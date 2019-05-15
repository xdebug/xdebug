--TEST--
Test for bug #1665: Segfault when overriding a function object parameter + collect_params > 0
--INI--
xdebug.default_enable = 1
xdebug.collect_params = 1
--FILE--
<?php

function query($var) {
	try {
		$var = "Country rooooooads"; // Rewriting an object var segfaults
		throw new LogicException('I am broken');
	} catch (Exception $ex) {
	}
}

query("What about a string?");
query(fopen("php://output", 'r'));
query(0);
query(3.1415);
query(array("Or", "an", "array", "of", 6, "elements?"));
query(false);
query(true);
query(null);

echo 'No segfault';
?>
--EXPECTF--
No segfault
