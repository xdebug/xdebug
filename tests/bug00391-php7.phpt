--TEST--
Test for bug #391: When PHP runs with Xdebug it doesn't stop executing script when type hinting leads to fatal error (>= PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '>=')) echo "skip >= PHP 7.0 needed\n"; ?>
--INI--
log_errors=0
xdebug.default_enable=1
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.collect_params=0
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
TypeError: Argument 1 passed to B::myMethod() must be an instance of A, integer given, called in %sbug00391-php7.php on line 17 in %sbug00391-php7.php on line 10

Call Stack:
%w%f%w%d   1. {main}() %sbug00391-php7.php:0
%w%f%w%d   2. B->myMethod() %sbug00391-php7.php:17


Fatal error: Uncaught TypeError: Argument 1 passed to B::myMethod() must be an instance of A, integer given, called in %sbug00391-php7.php on line 17 and defined in %sbug00391-php7.php on line 10

TypeError: Argument 1 passed to B::myMethod() must be an instance of A, integer given, called in %sbug00391-php7.php on line 17 in %sbug00391-php7.php on line 10

Call Stack:
%w%f%w%d   1. {main}() %sbug00391-php7.php:0
%w%f%w%d   2. B->myMethod() %sbug00391-php7.php:17
