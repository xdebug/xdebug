--TEST--
Check for vle presence
--SKIPIF--
<?php if (!extension_loaded("vle")) print "skip"; ?>
--POST--
--GET--
--FILE--
<?php 
echo "vle extension is available";
/*
	you can add regression tests for your extension here

  the output of your test code has to be equal to the
  text in the --EXPECT-- section below for the tests
  to pass, differences between the output and the
  expected text are interpreted as failure

	see php4/tests/README for further information on
  writing regression tests
*/
?>
--EXPECT--
vle extension is available