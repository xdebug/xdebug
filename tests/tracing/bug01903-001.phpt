--TEST--
Test for bug #1903: Tracing constants are only defined with mode=tracing
--INI--
xdebug.mode=develop
--FILE--
<?php
echo XDEBUG_TRACE_APPEND, ' ', XDEBUG_TRACE_COMPUTERIZED, ' ', XDEBUG_TRACE_HTML, "\n";
?>
--EXPECT--
1 2 4
