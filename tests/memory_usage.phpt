--TEST--
Test for xdebug.collect_params setting
--INI--
xdebug.enable=1
xdebug.collect_params=1
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.auto_trace=0
--FILE--
<?php
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 

	function foo ($a, $b, $c, $d, $e, $f) {
	}

	echo "Alive!\n";
	for ($i = 0; $i < 10000; $i++) {
		foo ($param, $param, $param, $param, $param, $param, $param, $param, $param, $param, $param, $param, $param, $param, $param, $param);
	}
	echo "Alive!\n";
?>
--EXPECT--
Alive!
Alive!
