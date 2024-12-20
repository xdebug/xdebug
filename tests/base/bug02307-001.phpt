--TEST--
Test for bug #2307: Segmentation fault due to a superglobal being a reference [1]
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
