--TEST--
Test for scream support.
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.scream=0
error_reporting(E_ALL);
--FILE--
<?php
echo @strstr(), "\n";
ini_set('xdebug.scream', 1);
echo @strstr(), "\n";
ini_set('xdebug.scream', 0);
echo @strstr(), "\n";
?>
--EXPECTF--
Warning: Wrong parameter count for strstr() in %sscream.php on line 4
