--TEST--
Test for bug #168: Memory error with DBGp eval when the result is an array
--SKIPIF--
<?php print "skip Can only be tested through DBGp"; ?>
--FILE--
<?php
$z = array("x", array("y1", "y2", "y3"), "z");
$x = "hello world.";
$y = "howdy world.";
print_r($z);
?>
--EXPECT--
