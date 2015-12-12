--TEST--
Test for bug #843: Text output depends on php locale. [normal]
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) == "WIN") die("skip Not for Windows");
if (false == setlocale(LC_ALL, "ro_RO.UTF-8", "de_DE.UTF-8", "de_DE", "de", "german", "ge", "de_DE.ISO-8859-1")) print "skip locale with , not found";
?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

setlocale(LC_ALL, "ro_RO.UTF-8", "de_DE.UTF-8", "de_DE", "de", "german", "ge", "de_DE.ISO-8859-1");

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d]
    %d.%d     %d     -> setlocale(6, 'ro_RO.UTF-8', 'de_DE.UTF-8', 'de_DE', 'de', 'german', 'ge', 'de_DE.ISO-8859-1') %sbug00843-002.php:4
    %d.%d     %d      >=> '%s'
    %d.%d     %d     -> xdebug_stop_trace() %sbug00843-002.php:6
    %d.%d     %d
TRACE END   [%d-%d-%d %d:%d:%d]
