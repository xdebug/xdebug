--TEST--
Check for xdebug presence
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--POST--
--GET--
--FILE--
<?php 
echo "xdebug extension is available";
?>
--EXPECT--
xdebug extension is available
