--TEST--
Test for bug #2034: Segmentation fault when 'set_time_limit' function is disabled
--INI--
disable_functions=set_time_limit
xdebug.mode=debug,develop
--FILE--
<?php
echo "Hello\n";
?>
--EXPECT--
Hello
