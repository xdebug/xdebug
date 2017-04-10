--TEST--
Test for bug #701: Functions as array indexes (PHP 5)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.collect_vars=1
xdebug.collect_params=4
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
TRACE START [%d-%d-%d %d:%d:%d]
                           => $tf = '%sxdt%s.xt' %sbug00701-php5.php:2
                           => $class = 'class' %sbug00701-php5.php:4
                           => $method = 'methodName' %sbug00701-php5.php:5
%w%f %w%d     -> ucfirst('class') %sbug00701-php5.php:6
%w%f %w%d      >=> 'Class'
                           => $action_ids[???]['methodName'] = 'methodName' %sbug00701-php5.php:6
%w%f %w%d     -> xdebug_stop_trace() %sbug00701-php5.php:8
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d]
