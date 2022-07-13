--TEST--
Test for bug #843: Text output depends on php locale [normal]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!win');
if (false == setlocale(LC_ALL, "ro_RO.UTF-8", "de_DE.UTF-8", "de_DE", "de", "german", "ge", "de_DE.ISO-8859-1")) print "skip locale not found";
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
require_once 'capture-trace.inc';

setlocale(LC_ALL, "ro_RO.UTF-8", "de_DE.UTF-8", "de_DE", "de", "german", "ge", "de_DE.ISO-8859-1");

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> setlocale($category = %d, $locales = 'ro_RO.UTF-8', ...$rest = variadic(0 => 'de_DE.UTF-8', 1 => 'de_DE', 2 => 'de', 3 => 'german', 4 => 'ge', 5 => 'de_DE.ISO-8859-1')) %sbug00843-002.php:4
%w%f %w%d      >=> '%s'
%w%f %w%d     -> xdebug_stop_trace() %sbug00843-002.php:6
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
