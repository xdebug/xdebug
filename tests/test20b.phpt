--TEST--
Test for static method calls
--SKIPIF--
<?php if(version_compare(zend_version(), "2.0.0-dev", '<')) echo "skip Zend Engine 2 needed\n"; ?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.auto_profile=0
--FILE--
<?php
$tf = xdebug_start_trace(tempnam('/tmp', 'xdt'));
class DB {
	static function query($s) {
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
    %f      %d     -> DB::query('test') /dat/dev/php/xdebug/tests/test20b.php:9
    %f      %d     -> file_get_contents('/tmp/%s') /%s/test20b.php:11
