--TEST--
Test for segmentation fault with unusual variables
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
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

Notice: Array to string conversion in %sbug00032.php on line %d

Notice: Array to string conversion in %sbug00032.php on line %d
foo

Notice: Object to string conversion in %sbug00032.php on line %d

Notice: Object to string conversion in %sbug00032.php on line %d
foo
