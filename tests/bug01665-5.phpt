--TEST--
Test for bug #1665: Segfault when overriding a function object parameter + collect_params > 0
--INI--
xdebug.default_enable = 1
xdebug.collect_params = 5
--FILE--
<?php

function query($var) {
	try {
		$var = "Country rooooooads"; // Rewriting an object var segfaults
		throw new LogicException('I am broken');
	} catch (Exception $ex) {
	}
}

query(new stdClass());
echo 'No segfault';
?>
--EXPECTF--
No segfault
