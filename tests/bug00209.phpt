--TEST--
Test for bug #209: Additional remote debugging session started when triggering shutdown function
--SKIPIF--
<?php print "skip Can only be tested through DBGp"; ?>
--FILE--
<?php
include("bug00209.inc");
echo "test";
?>
--EXPECT--
