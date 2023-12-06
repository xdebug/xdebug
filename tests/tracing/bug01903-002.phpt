--TEST--
Test for bug #1903: Tracing constants should be defined with mode=off
--INI--
xdebug.mode=off
--FILE--
<?php
var_dump(
	defined( 'XDEBUG_TRACE_APPEND' ),
	defined( 'XDEBUG_TRACE_COMPUTERIZED' ),
	defined( 'XDEBUG_TRACE_HTML' )
);
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
