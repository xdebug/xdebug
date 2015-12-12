--TEST--
Test for bug #1112: Setting an invalid xdebug.trace_format causes Xdebug to crash
--SKIPIF--
<?php if (substr(PHP_OS, 0, 3) == "WIN") die("skip Not for Windows"); ?>
--INI--
xdebug.trace_format=42
xdebug.auto_trace=1
xdebug.default_enable=1
log_errors=1
--FILE--
<?php
echo strlen("45"), "\n";
?>
--EXPECT--
PHP Notice:  A wrong value for xdebug.trace_format was selected (42), defaulting to the textual format. in Unknown on line 0
2
