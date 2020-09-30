--TEST--
Test for bug #701: Functions as array indexes
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.trace_format=0
xdebug.collect_return=1
xdebug.collect_assignments=1
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

$class = "class";
$method = "methodName";
$action_ids[ucfirst($class)][$method] = $method;

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                           => $tf = '%sxdt%s.xt' %sbug00701.php:2
                           => $class = 'class' %sbug00701.php:4
                           => $method = 'methodName' %sbug00701.php:5
%w%f %w%d     -> ucfirst($str%S = 'class') %sbug00701.php:6
%w%f %w%d      >=> 'Class'
                           => $action_ids['Class']['methodName'] = 'methodName' %sbug00701.php:6
%w%f %w%d     -> xdebug_stop_trace() %sbug00701.php:8
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
