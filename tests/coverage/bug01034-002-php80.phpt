--TEST--
Test for bug #1034: path coverage [2] (>= PHP 8.0; !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0; !opcache');
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
  - 00; OP: 00-02; line: 02-02 HIT; out1: 08 HIT
  - 03; OP: 03-07; line: 03-02 HIT; out1: 08 HIT
  - 08; OP: 08-10; line: 02-02 HIT; out1: 11 HIT; out2: 03 HIT
  - 11; OP: 11-13; line: 05-07 HIT; out1: EX  X 
- paths
  - 0 8 11:  X 
  - 0 8 3 8 11: HIT
