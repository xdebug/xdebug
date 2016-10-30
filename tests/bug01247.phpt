--TEST--
Test for bug #1247: xdebug.show_local_vars dumps variables with *uninitialized* values
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
}

function testDirect()
{
	$c = 56;

	trigger_error('test');

	$d = 11;
}

test();
testDirect();
?>
--EXPECTF--
Warning: strtr() expects at least 2 parameters, 0 given in %sbug01247.php on line 6

Call Stack:
%w%f %w%d   1. {main}() %sbug01247.php:0
%w%f %w%d   2. test() %sbug01247.php:20
%w%f %w%d   3. strtr() %sbug01247.php:6


Variables in local scope (#2):
  $a = 42
  $b = *uninitialized*


Notice: test in %sbug01247.php on line 15

Call Stack:
%w%f %w%d   1. {main}() %sbug01247.php:0
%w%f %w%d   2. testDirect() %sbug01247.php:21
%w%f %w%d   3. trigger_error('test') %sbug01247.php:15


Variables in local scope (#2):
  $c = 56
  $d = *uninitialized*
