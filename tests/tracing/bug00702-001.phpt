--TEST--
Test for bug #702: Check whether variables tracing also works with =&
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.trace_format=0
xdebug.dump_globals=0
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.force_error_reporting=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

$a = 42;
$b =& $a;
$b = 43;

$array = [ 1, 2 ];
$array[] = 3;
$array[] =& $array[2];

$array = [ 1, 2, [ 3, 4, 6 ] ];
$array[] = $array[2][1];
$array[] =& $array[2][1];

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                           => $tf = '%s.xt' %sbug00702-001.php:2
                           => $a = 42 %sbug00702-001.php:4
                           => $b =& $a %sbug00702-001.php:5
                           => $b = 43 %sbug00702-001.php:6
                           => $array = [0 => 1, 1 => 2] %sbug00702-001.php:8
                           => $array[] = 3 %sbug00702-001.php:9
                           => $array[] =& $array[2] %sbug00702-001.php:10
                           => $array = [0 => 1, 1 => 2, 2 => [0 => 3, 1 => 4, 2 => 6]] %sbug00702-001.php:12
                           => $array[] = 4 %sbug00702-001.php:13
                           => $array[] =& $array[2][1] %sbug00702-001.php:14
%w%f %w%d     -> xdebug_stop_trace() %sbug00702-001.php:16
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
