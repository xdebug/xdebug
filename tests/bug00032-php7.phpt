--TEST--
Test for segmentation fault with unusual variables (>= PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '>=')) echo "skip >= PHP 7.0 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_assignments=0
xdebug.profiler_enable=0
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
Notice: Array to string conversion in %sbug00032-php7.php on line 8


Notice: Array to string conversion in %sbug00032-php7.php on line 9

foo
foo
foo

Catchable fatal error: Object of class stdClass could not be converted to string in %sbug00032-php7.php on line 11

Call Stack:
%w%f %w%d   1. {main}() %sbug00032-php7.php:0
