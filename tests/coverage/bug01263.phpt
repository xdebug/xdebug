--TEST--
Test for bug #1263: Coverage of sending arguments to a method (opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('opcache');
?>
--INI--
xdebug.mode=coverage
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
