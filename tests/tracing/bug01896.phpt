--TEST--
Test for bug #1896: Crash with Closure::fromCallable
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=0
xdebug.collect_return=1
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/'. uniqid('xdt', TRUE));

$a = new \ReflectionClass(\stdClass::class);
$b = \Closure::fromCallable([$a, 'newInstanceWithoutConstructor']);
$c = $b();

xdebug_stop_trace();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> ReflectionClass->__construct($%s = 'stdClass') %sbug01896.php:4
%w%f %w%d      >=> NULL
%w%f %w%d     -> Closure::fromCallable($call%s = [0 => class ReflectionClass { public %S$name = 'stdClass' }, 1 => 'newInstanceWithoutConstructor']) %sbug01896.php:5
%w%f %w%d      >=> class Closure { virtual $closure = "$this->newInstanceWithoutConstructor", public $this = class ReflectionClass { public %S$name = 'stdClass' } }
%w%f %w%d     -> ReflectionClass->newInstanceWithoutConstructor() %sbug01896.php:6
%w%f %w%d      >=> class stdClass {  }
%w%f %w%d     -> xdebug_stop_trace() %sbug01896.php:8
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
