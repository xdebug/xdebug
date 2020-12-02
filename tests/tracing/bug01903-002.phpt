--TEST--
Test for bug #1903: Tracing constants should not be defined with mode=off
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
bool(false)
bool(false)
bool(false)
