--TEST--
Test for static method calls
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
--FILE--
<?php
xdebug_start_trace($tf = tempnam('/tmp', 'xdt'));
class DB {
	function query($s) {
		echo $s."\n";
	}
}

DB::query("test");

echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
test

TRACE START [%d-%d-%d %d:%d:%d]
    %f      %d     -> db::query('test') /dat/dev/php/xdebug/tests/test20.php:9
    %f      %d     -> file_get_contents('/tmp/%s') /%s/test20.php:11
