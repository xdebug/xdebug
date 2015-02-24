--TEST--
Test for bug #1112: Setting an invalid xdebug.trace_format causes Xdebug to crash
--INI--
xdebug.trace_format=42
xdebug.auto_trace=1
xdebug.default_enable=1
log_errors=1
--FILE--
<?php
echo strlen("45");
?>
--EXPECT--
PHP Notice:  A wrong value for xdebug.trace_format was selected (42), defaulting to the textual format. in Unknown on line 0
2
