<?php
class ExceptionFoo extends Exception {}
class ExceptionBar extends Exception {}
class ExceptionBaz extends Exception {}

function trycatch($action)
{
	echo "Let's do some stuff!\n";
	$f = 5 * pow(3, 7);

	try {
		switch ($action) {
			case 1: throw new ExceptionFoo("test");
			case 2: throw new ExceptionBaz("test");
			case 3: throw new Exception("test");
		}
		echo "Not thrown\n";
	} catch (ExceptionFoo $e) {
		echo "caught\n";
	} catch (ExceptionBar $e) {
		echo "caught\n";
	} catch (ExceptionBaz $e) {
		echo "caught\n";
	}

	echo "And do some more\n";
}

trycatch(1);
trycatch(2);
try {
	trycatch(3);
} catch ( Exception $e ) {
	echo "CAUGHT IN MAIN\n";
}
?>
