--TEST--
Test for bug #843: Text output depends on php locale [computerized]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!win');
if (false == setlocale(LC_ALL, "ro_RO.UTF-8", "de_DE.UTF-8", "de_DE", "de", "german", "ge", "de_DE.ISO-8859-1")) print "skip locale not found";
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.trace_format=1
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

setlocale(LC_ALL, "ro_RO.UTF-8", "de_DE.UTF-8", "de_DE", "de", "german", "ge", "de_DE.ISO-8859-1");

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
Version: %d.%s
File format: %d
TRACE START [%d-%d-%d %d:%d:%d.%d]
2	4	0	%d.%d	%d	setlocale	0		%sbug00843-001.php	4	8	%d	'ro_RO.UTF-8'	'de_DE.UTF-8'	'de_DE'	'de'	'german'	'ge'	'de_DE.ISO-8859-1'
2	4	1	%d.%d	%d
2	4	R			'%s'
2	5	0	%d.%d	%d	xdebug_stop_trace	0		%sbug00843-001.php	6	0
			%d.%d	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
