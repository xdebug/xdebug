--TEST--
Check for xdebug presence
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--INI--
xdebug.profiler_enable=0
--FILE--
<?php 
echo "xdebug extension is available";
?>
--EXPECT--
xdebug extension is available
