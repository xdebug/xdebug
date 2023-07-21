--TEST--
Test for bug #315: Xdebug crashes when including a file that doesn't exist
--INI--
xdebug.mode=develop
xdebug.dump_globals=0
xdebug.trace_format=0
xdebug.force_error_reporting=0
--FILE--
<?php
@include 'this-file-does-not-exist.php';
echo "ALIVE\n";
?>
--EXPECT--
ALIVE
