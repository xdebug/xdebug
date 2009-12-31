--TEST--
Test for segmentation fault with unusual variables (ZE2.2)
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if(version_compare(zend_version(), "2.2.0-dev", '<')) echo "skip Zend Engine 2.2 needed\n"; ?>
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
foo
foo

Notice: Array to string conversion in %sbug00032-ze22.php on line 8

Call Stack:
%w%f %w%d   1. {main}() %sbug00032-ze22.php:0


Notice: Array to string conversion in %sbug00032-ze22.php on line 9

Call Stack:
%w%f %w%d   1. {main}() %sbug00032-ze22.php:0

foo

Catchable fatal error: Object of class stdClass could not be converted to string in %sbug00032-ze22.php on line 11

Call Stack:
%w%f %w%d   1. {main}() %sbug00032-ze22.php:0
