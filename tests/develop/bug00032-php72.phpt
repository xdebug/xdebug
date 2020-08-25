--TEST--
Test for segmentation fault with unusual variables (< PHP 7.4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.4');
?>
--INI--
xdebug.mode=develop
xdebug.collect_assignments=0
xdebug.show_local_vars=0
xdebug.dump_globals=0
--FILE--
<?php
	${1} = "foo";
	echo ${1} . "\n";

	${STDIN} = "foo";
	echo ${STDIN} . "\n";

	${array(1,2,3)} = "foo";
	echo ${array(1,2,3)} . "\n";

	${new stdclass} = "foo";
	echo ${new stdclass} . "\n";
?>
--EXPECTF--
Notice: Array to string conversion in %sbug00032-php72.php on line 8


Notice: Array to string conversion in %sbug00032-php72.php on line 9

foo
foo
foo

Recoverable fatal error: Object of class stdClass could not be converted to string in %sbug00032-php72.php on line 11

Call Stack:
%w%f %w%d   1. {main}() %sbug00032-php72.php:0
