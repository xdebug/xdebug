--TEST--
Test for bug #318: Segmentation Fault in code coverage analysis
--INI--
xdebug.mode=coverage
--FILE--
<?php
// Run me from the PHP CLI
xdebug_start_code_coverage(XDEBUG_CC_DEAD_CODE | XDEBUG_CC_UNUSED);
// MUST be both code coverage options to cause problems
include(dirname(__FILE__).'/bug00318.inc'); // File with problem in it.
xdebug_stop_code_coverage();
?>
--EXPECTF--
Fatal error: %sbreak%s in %sbug00318.inc on line 3
