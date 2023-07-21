--TEST--
Test stack traces (text)
--INI--
xdebug.mode=develop
xdebug.trace_format=0
xdebug.dump_globals=0
xdebug.var_display_max_children=50
xdebug.var_display_max_depth=5
xdebug.var_display_max_length=64
xdebug.collect_return=0
xdebug.show_local_vars=0
xdebug.show_error_trace=1
--FILE--
<?php
function foo( $a ) {
    for ($i = 1; $i < $a['foo']; $i++) {
		poo();
    }
}

$c = new stdClass;
$c->bar = 100;
$a = array(
    42 => false, 'foo' => 912124,
    $c, new stdClass, fopen( __FILE__, 'r' )
);
try { foo( $a ); } catch (Throwable $e) { /* ignore */ }
?>
--EXPECTF--
%srror: Call to undefined function poo() in %sstacktrace%s.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sstacktrace%s.php:0
%w%f %w%d   2. foo($a = [42 => FALSE, 'foo' => 912124, 43 => class stdClass { public $bar = 100 }, 44 => class stdClass {  }, 45 => resource(%d) of type (stream)]) %sstacktrace%s.php:14
