--TEST--
Test for bug #1247: xdebug.show_local_vars dumps variables with *uninitialized* values (opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('opcache');
?>
--INI--
xdebug.mode=develop
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
	hex2bin('4');
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
Warning: hex2bin(): %s in %sbug01247-opcache.php on line 5

Call Stack:
%w%f %w%d   1. {main}() %sbug01247-opcache.php:0
%w%f %w%d   2. test() %sbug01247-opcache.php:20
%w%f %w%d   3. hex2bin($%s = '4') %sbug01247-opcache.php:5


Notice: test in %sbug01247-opcache.php on line 14

Call Stack:
%w%f %w%d   1. {main}() %sbug01247-opcache.php:0
%w%f %w%d   2. testDirect() %sbug01247-opcache.php:21
%w%f %w%d   3. trigger_error($message = 'test') %sbug01247-opcache.php:14
