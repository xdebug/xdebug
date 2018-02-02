--TEST--
Test for bug #1263: Coverage of sending arguments to a method (>= PHP 7.0.8)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0.8", '>=')) echo "skip >= PHP 7.0.8 needed\n"; ?>
<?php if (!extension_loaded('zend opcache')) echo "skip opcache required\n"; ?>
--INI--
opcache.enable_cli=1
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

include 'bug01263.inc';

xdebug_stop_code_coverage(false);

?>
==DONE==
--EXPECT--
==DONE==
