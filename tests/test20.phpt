--TEST--
Test for static method calls
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if(version_compare(zend_version(), "2.0.0-dev", '>=')) echo "skip Zend Engine 1 needed\n"; ?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));
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
%w%f %w%d     -> db::query('test') /dat/dev/php/xdebug/tests/test20.php:9
%w%f %w%d     -> file_get_contents('/tmp/%s') /%s/test20.php:11
