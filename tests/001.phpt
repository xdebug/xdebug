--TEST--
Check for xdebug presence
--INI--
xdebug.profiler_enable=0
--FILE--
<?php 
echo "xdebug extension is available";
?>
--EXPECT--
xdebug extension is available
