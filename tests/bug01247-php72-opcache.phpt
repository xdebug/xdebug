--TEST--
Test for bug #1247: xdebug.show_local_vars dumps variables with *uninitialized* values (>= PHP 7.2 && opcache)
--SKIPIF--
<?php
if ( ! ( version_compare(phpversion(), "7.2", '>=') && extension_loaded('zend opcache'))) { echo "skip >= PHP 7.2 && opcache loaded needed\n"; };
?>
--INI--
xdebug.default_enable=1
xdebug.collect_params=4
html_errors=0
xdebug.dump.GET=
xdebug.dump.POST=
xdebug.show_local_vars=1
--GET--
getFoo=bar
--POST--
postBar=baz
--FILE--
<?php
function test()
{
	$a = 42;
	strtr();
	$b = 4;

	return $a + $b;
}

function testDirect()
{
	$c = 56;
	trigger_error('test');
	$d = 11;

	return $c + $d;
}

test();
testDirect();
?>
--EXPECTF--
Warning: strtr() expects at least 2 parameters, 0 given in %sbug01247-php72-opcache.php on line 5

Call Stack:
%w%f %w%d   1. {main}() %sbug01247-php72-opcache.php:0
%w%f %w%d   2. test() %sbug01247-php72-opcache.php:20
%w%f %w%d   3. strtr() %sbug01247-php72-opcache.php:5


Notice: test in %sbug01247-php72-opcache.php on line 14

Call Stack:
%w%f %w%d   1. {main}() %sbug01247-php72-opcache.php:0
%w%f %w%d   2. testDirect() %sbug01247-php72-opcache.php:21
%w%f %w%d   3. trigger_error('test') %sbug01247-php72-opcache.php:14
