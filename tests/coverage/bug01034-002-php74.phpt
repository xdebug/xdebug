--TEST--
Test for bug #1034: path coverage [2] (< PHP 8.0; !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 8.0; !opcache');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

include 'bug01034-002.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
?>
--EXPECTF--
0 1 2 3 4 
{main}
- branches
  - 00; OP: 00-02; line: 02-02 HIT; out1: 09 HIT
  - 03; OP: 03-08; line: 03-02 HIT; out1: 09 HIT
  - 09; OP: 09-11; line: 02-02 HIT; out1: 12 HIT; out2: 03 HIT
  - 12; OP: 12-14; line: 05-07 HIT; out1: EX  X 
- paths
  - 0 9 12:  X 
  - 0 9 3 9 12: HIT
