--TEST--
Test for xdebug_peak_memory_usage
--INI--
xdebug.mode=develop
xdebug.auto_profile=0
--FILE--
<?php
	$a = xdebug_memory_usage();

	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2)))))))))))); 
	$param[] = array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, array (1, 2))))))))))));

	$b = xdebug_memory_usage();
	$c = xdebug_peak_memory_usage();

	unset($param);

	$e = xdebug_peak_memory_usage();
	$d = xdebug_memory_usage();

	var_dump($a, $b, $c, $d, $e);
	echo ($b > $c) ? "Current is HIGHER than peak\n" : "Current is lower than peak\n";
	echo ($d > $e) ? "Current is HIGHER than peak\n" : "Current is lower than peak\n";
?>
--EXPECTF--
%speak_memory_usage.php:22:
int(%d)
%speak_memory_usage.php:22:
int(%d)
%speak_memory_usage.php:22:
int(%d)
%speak_memory_usage.php:22:
int(%d)
%speak_memory_usage.php:22:
int(%d)
Current is lower than peak
Current is lower than peak
