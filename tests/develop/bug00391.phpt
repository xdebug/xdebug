--TEST--
Test for bug #391: When PHP runs with Xdebug it doesn't stop executing script when type hinting leads to fatal error
--INI--
log_errors=0
xdebug.mode=develop
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
xdebug.show_error_trace=1
--FILE--
<?php

class A
{
	public $x = 1;
}

class B
{
	public function myMethod(A $y)
	{
		echo $y, "\n";
	}
}

$z = new B();
$z->myMethod(123);
echo "And going and going...\n";
?>
DONE
--EXPECTF--
TypeError: %SA, int%S given, called in %sbug00391.php on line 17 in %sbug00391.php on line 10

Call Stack:
%w%f%w%d   1. {main}() %sbug00391.php:0
%w%f%w%d   2. B->myMethod($y = 123) %sbug00391.php:17


Fatal error: %SA, int%S given, called in %sbug00391.php on line 17 and defined in %sbug00391.php on line 10

TypeError: %SA, int%S given, called in %sbug00391.php on line 17 in %sbug00391.php on line 10

Call Stack:
%w%f%w%d   1. {main}() %sbug00391.php:0
%w%f%w%d   2. B->myMethod($y = 123) %sbug00391.php:17
