--TEST--
Test for bug #702: Check whether variables tracing also works with =& (with object)
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=0
xdebug.dump_globals=0
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.force_error_reporting=0
--FILE--
<?php
require_once 'capture-trace.inc';

$a = new stdClass;
$b =& $a;
$b = 43;

$object = new stdClass;
$object->foo = 'bar';
$object->array = [ 1, 2, 3, 5, 8, 13 ];
$object->bar =& $object->foo;
$object->array[] =& $object->foo;
$object->array[] =& $object->array[4];

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
                             => $tf = '%sxt%S' %s:%d
                           => $a = class stdClass {  } %sbug00702-002.php:4
                           => $b =& $a %sbug00702-002.php:5
                           => $b = 43 %sbug00702-002.php:6
                           => $object = class stdClass {  } %sbug00702-002.php:8
                           => $object->foo = 'bar' %sbug00702-002.php:9
                           => $object->array = [0 => 1, 1 => 2, 2 => 3, 3 => 5, 4 => 8, 5 => 13] %sbug00702-002.php:10
                           => $object->bar =& $object->foo %sbug00702-002.php:11
                           => $object->array[] =& $object->foo %sbug00702-002.php:12
                           => $object->array[] =& $object->array[4] %sbug00702-002.php:13
%w%f %w%d     -> xdebug_stop_trace() %sbug00702-002.php:15
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
