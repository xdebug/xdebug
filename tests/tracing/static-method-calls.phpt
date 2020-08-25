--TEST--
Test for static method calls
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));
class DB {
	static function query($s) {
		echo $s."\n";
	}
}

DB::query("test");

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
test
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> DB::query($s = 'test') %sstatic-method-calls.php:9
%w%f %w%d     -> xdebug_stop_trace() %sstatic-method-calls.php:11
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
