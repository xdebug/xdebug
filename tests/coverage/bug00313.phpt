--TEST--
Test for bug #313: Segmentation Fault in code coverage analysis on empty PHP files
--INI--
xdebug.mode=coverage
xdebug.dump_globals=0
xdebug.trace_format=0
--FILE--
<?php
// Run me from the PHP CLI
xdebug_start_code_coverage(XDEBUG_CC_DEAD_CODE | XDEBUG_CC_UNUSED);
// MUST be both code coverage options to cause problems
include(dirname(__FILE__).'/bug00313.inc'); // File with problem in it.
xdebug_stop_code_coverage();
echo "ALIVE\n";
?>
--EXPECTF--
ALIVE
