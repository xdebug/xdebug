--TEST--
Test for bug #1112: Setting an invalid xdebug.trace_format causes Xdebug to crash
--SKIPIF--
<?php if (substr(PHP_OS, 0, 3) == "WIN") die("skip Not for Windows"); ?>
--INI--
xdebug.trace_format=42
xdebug.auto_trace=1
xdebug.default_enable=1
log_errors=0
--FILE--
<?php
echo strlen("45"), "\n";
?>
--EXPECTF--
Notice: A wrong value for xdebug.trace_format was selected (42), defaulting to the textual format in %s/bug01112.php on line 2

2
