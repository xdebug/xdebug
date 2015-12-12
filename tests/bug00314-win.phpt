--TEST--
Test for bug #314: PHP CLI Error logging thwarted when Xdebug loaded (Windows)
--SKIPIF--
<?php if (substr(PHP_OS, 0, 3) != "WIN") { exit("skip For Windows only"); } ?>
--INI--
xdebug.default_enable=1
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.profiler_enable=0
xdebug.trace_format=0
log_errors=1
error_log=C:\Windows\Temp\bug314-win.log
xdebug.collect_params=3
date.timezone=UTC
--FILE--
<?php
@unlink('C:\Windows\Temp\bug314-win.log');
trigger_error('Error', E_USER_WARNING);
echo "FROM LOG\n";
echo file_get_contents('C:\Windows\Temp\bug314-win.log');
?>
--EXPECTF--
Warning: Error in %sbug00314-win.php on line 3

Call Stack:
%w%f %w%d   1. {main}() %sbug00314-win.php:0
%w%f %w%d   2. trigger_error('Error', 512) %sbug00314-win.php:3

FROM LOG
[%d-%s-%d %d:%d:%d%s] PHP Warning:  Error in %sbug00314-win.php on line 3
[%d-%s-%d %d:%d:%d%s] PHP Stack trace:
[%d-%s-%d %d:%d:%d%s] PHP   1. {main}() %sbug00314-win.php:0
[%d-%s-%d %d:%d:%d%s] PHP   2. trigger_error('Error', 512) %sbug00314-win.php:3
