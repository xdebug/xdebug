--TEST--
Test for static method calls
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
--FILE--
<?php
xdebug_start_trace();
class DB {
	function query($s) {
		echo $s."\n";
	}
}

DB::query("test");

xdebug_dump_function_trace();
?>
--EXPECTF--
test

Function trace:
    %f      %d     -> db::query('test') /dat/dev/php/xdebug/tests/test20.php:9
