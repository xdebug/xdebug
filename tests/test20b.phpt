--TEST--
Test for static method calls
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
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
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> DB::query('test') %stest20b.php:9
%w%f %w%d     -> xdebug_stop_trace() %stest20b.php:11
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
