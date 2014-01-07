--TEST--
Test for bug #315: Xdebug crashes when including a file that doesn't exist
--INI--
xdebug.default_enable=1
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.profiler_enable=0
xdebug.trace_format=0
xdebug.force_error_reporting=0
--FILE--
<?php
@include 'this-file-does-not-exist.php';
echo "ALIVE\n";
?>
--EXPECT--
ALIVE
