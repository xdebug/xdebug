--TEST--
Test for bug #1571: Stack traces don't show file/line for closures in namespaces (>= PHP 8.4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.4');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
namespace A;

require_once 'capture-trace.inc';

$a = function(){

};
$a(1, $a);

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> {closure:%sbug01571-001-php84.php:6-8}(1, class Closure { public $name = '{closure:%sbug01571-001-php84.php:6}'; public $file = '%sbug01571-001-php84.php'; public $line = 6 }) %sbug01571-001-php84.php:9
%w%f %w%d     -> xdebug_stop_trace() %sbug01571-001-php84.php:11
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
