--TEST--
Test for bug #2307: Segmentation fault due to a superglobal being a reference [3]
--INI--
xdebug.mode=develop,trace
--FILE--
<?php
$get = &$_GET;
function bla(Throwable $exception) {
echo "fasel";
}
set_exception_handler("bla");
include __DIR__ . '/bug02307-003.inc';
?>
--EXPECT--
fasel
