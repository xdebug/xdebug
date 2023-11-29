--TEST--
Test for bug #2164: path/branch coverage for first class callable
--XFAIL--
Bug #2164: path/branch coverage for first class callable
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

include 'bug02164.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
var_dump($c);
dump_branch_coverage($c);
?>
--EXPECTF--
