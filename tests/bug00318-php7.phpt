--TEST--
Test for bug #318: Segmentation Fault in code coverage analysis (>= PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '>=')) echo "skip >= PHP 7.0 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.profiler_enable=0
xdebug.trace_format=0
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

Call Stack:
%w%f %w%d   1. {main}() %sbug00318-php7.php:0
