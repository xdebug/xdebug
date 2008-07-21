--TEST--
Test for bug #314: PHP CLI Error Logging thwarted when XDebug Loaded.
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--INI--
xdebug.default_enable=1
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.profiler_enable=0
xdebug.trace_format=0
log_errors=1
error_log=/tmp/bug315.log
xdebug.collect_params=3
--FILE--
<?php
@unlink("/tmp/bug315.log");
trigger_error('Error', E_USER_WARNING);
echo "FROM LOG\n";
echo file_get_contents("/tmp/bug315.log");
?>
--EXPECTF--
Warning: Error in %sbug00314.php on line 3

Call Stack:
%w%f %w%d   1. {main}() %sbug00314.php:0
%w%f %w%d   2. trigger_error('Error', 512) %sbug00314.php:3

FROM LOG
[%d-%s-%d %d:%d:%d] PHP Warning:  Error in %sbug00314.php on line 3
[%d-%s-%d %d:%d:%d] PHP Stack trace:
[%d-%s-%d %d:%d:%d] PHP   1. {main}() %sbug00314.php:0
[%d-%s-%d %d:%d:%d] PHP   2. trigger_error('Error', 512) %sbug00314.php:3
