--TEST--
Test for bug #1266: xdebug_dump_superglobals() always dumps empty $_SESSION stack on PHP 7
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if (!extension_loaded("session")) print "skip session extension not available"; ?>
--INI--
xdebug.dump.SESSION=*
--FILE--
<?php
session_start();
$_SESSION['foo'] = 'bar';

xdebug_dump_superglobals();
?>
--EXPECT--
Dump $_SESSION
   $_SESSION['foo'] = 'bar'
