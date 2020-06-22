--TEST--
Test for bug #1112: Setting an invalid xdebug.trace_format causes Xdebug to crash
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!win');
?>
--INI--
xdebug.mode=trace,develop
xdebug.start_with_request=yes
xdebug.trace_format=42
log_errors=0
--FILE--
<?php
echo strlen("45"), "\n";
?>
--EXPECTF--
Notice: A wrong value for xdebug.trace_format was selected (42), defaulting to the textual format in %s/bug01112.php on line 2

2
