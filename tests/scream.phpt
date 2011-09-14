--TEST--
Test for scream support
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
Warning:%sstrstr()%sin %sscream.php on line 4
