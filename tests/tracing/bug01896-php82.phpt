--TEST--
Test for bug #1896: Crash with Closure::fromCallable (>= PHP 8.2)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.2');
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

$a = new \ReflectionClass(\stdClass::class);
$b = \Closure::fromCallable([$a, 'newInstanceWithoutConstructor']);
$c = $b();

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> ReflectionClass->__construct($%s = 'stdClass') %sbug01896-php82.php:4
%w%f %w%d      >=> NULL
%w%f %w%d     -> Closure::fromCallable($call%s = [0 => class ReflectionClass { public %S$name = 'stdClass' }, 1 => 'newInstanceWithoutConstructor']) %sbug01896-php82.php:5
%w%f %w%d      >=> class Closure { public $function = 'ReflectionClass::newInstanceWithoutConstructor'; public $this = class ReflectionClass { public string $name = 'stdClass' } }
%w%f %w%d     -> ReflectionClass->newInstanceWithoutConstructor() %sbug01896-php82.php:6
%w%f %w%d      >=> class stdClass {  }
%w%f %w%d     -> xdebug_stop_trace() %sbug01896-php82.php:8
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
