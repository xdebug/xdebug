--TEST--
Test for segmentation fault with unusual variables
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
%s: Array to string conversion in %sbug00032.php on line 8


%s: Array to string conversion in %sbug00032.php on line 9

foo
foo
foo

Fatal error: Uncaught Error: Object of class stdClass could not be converted to string in %sbug00032.php on line 11

Error: Object of class stdClass could not be converted to string in %sbug00032.php on line 11

Call Stack:
%w%f %w%d   1. {main}() %sbug00032.php:0
