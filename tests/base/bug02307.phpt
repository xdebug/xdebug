--TEST--
Test for bug #2307: Segmentation fault during shutdown
--INI--
xdebug.mode=develop,trace
--FILE--
<?php
echo "bla";
$fasel = &$_GET;
register_shutdown_function(function () { echo "shutdown"; });
?>
--EXPECT--
blashutdown
