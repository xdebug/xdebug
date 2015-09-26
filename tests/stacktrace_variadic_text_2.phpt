--TEST--
Test for stack traces with variadics (text, 2)
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.6", '>=')) echo "skip >= PHP 5.6 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.profiler_enable=0
xdebug.auto_trace=0
xdebug.trace_format=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.collect_vars=0
xdebug.collect_params=2
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
--FILE--
<?php
function foo( $a, ...$b )
{
	trigger_error( 'notice' );
}

foo( 42 );
foo( 1, false );
foo( "foo", "bar", 3.1415 );
?>
--EXPECTF--
Notice: notice in %sstacktrace_variadic_text_2.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sstacktrace_variadic_text_2.php:0
%w%f %w%d   2. foo(long, ...variadic()) %sstacktrace_variadic_text_2.php:7
%w%f %w%d   3. trigger_error(string(6)) %sstacktrace_variadic_text_2.php:4


Notice: notice in %sstacktrace_variadic_text_2.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sstacktrace_variadic_text_2.php:0
%w%f %w%d   2. foo(long, ...variadic(%r(bool|false)%r)) %sstacktrace_variadic_text_2.php:8
%w%f %w%d   3. trigger_error(string(6)) %sstacktrace_variadic_text_2.php:4


Notice: notice in %sstacktrace_variadic_text_2.php on line 4

Call Stack:
%w%f %w%d   1. {main}() %sstacktrace_variadic_text_2.php:0
%w%f %w%d   2. foo(string(3), ...variadic(string(3), double)) %sstacktrace_variadic_text_2.php:9
%w%f %w%d   3. trigger_error(string(6)) %sstacktrace_variadic_text_2.php:4
