--TEST--
Test for stack traces with variadics (error_log)
--INI--
xdebug.mode=develop
xdebug.trace_format=0
xdebug.dump_globals=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.force_error_reporting=0
log_errors=1
error_log=
display_errors=0
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
PHP Notice:  notice in %serrorlog_variadic.php on line 4
PHP Stack trace:
PHP   1. {main}() %serrorlog_variadic.php:0
PHP   2. foo($a = 42) %serrorlog_variadic.php:7
PHP   3. trigger_error($message = 'notice') %serrorlog_variadic.php:4
PHP Notice:  notice in %serrorlog_variadic.php on line 4
PHP Stack trace:
PHP   1. {main}() %serrorlog_variadic.php:0
PHP   2. foo($a = 1, ...$b = variadic(FALSE)) %serrorlog_variadic.php:8
PHP   3. trigger_error($message = 'notice') %serrorlog_variadic.php:4
PHP Notice:  notice in %serrorlog_variadic.php on line 4
PHP Stack trace:
PHP   1. {main}() %serrorlog_variadic.php:0
PHP   2. foo($a = 'foo', ...$b = variadic('bar', 3.1415)) %serrorlog_variadic.php:9
PHP   3. trigger_error($message = 'notice') %serrorlog_variadic.php:4
