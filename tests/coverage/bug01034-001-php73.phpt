--TEST--
Test for bug #1034: path coverage [1] (>= PHP 7.3, < PHP 7.4, !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.3,< 7.4; !opcache');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

include 'bug01034-001.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
?>
--EXPECTF--
0 1 2 3 
!42
caught
ifelse
- branches
  - 00; OP: 00-04; line: 10-12 HIT; out1: 05  X ; out2: 08 HIT
  - 05; OP: 05-07; line: 13-13  X ; out1: 12  X 
  - 08; OP: 08-11; line: 15-16 HIT; out1: EX  X 
  - 12; OP: 12-15; line: 18-19  X ; out1: EX  X 
- paths
  - 0 5 12:  X 
  - 0 8: HIT

loopy
- branches
  - 00; OP: 00-04; line: 02-04 HIT; out1: 11 HIT
  - 05; OP: 05-10; line: 05-04 HIT; out1: 11 HIT
  - 11; OP: 11-13; line: 04-04 HIT; out1: 14 HIT; out2: 05 HIT
  - 14; OP: 14-17; line: 07-08 HIT; out1: EX  X 
- paths
  - 0 11 14: HIT
  - 0 11 5 11 14: HIT

trycatch
- branches
  - 00; OP: 00-08; line: 21-24 HIT; out1: EX  X 
  - 12; OP: 12-12; line: 26-26 HIT; out1: 13 HIT; out2: EX  X 
  - 13; OP: 13-16; line: 27-29 HIT; out1: EX  X 
- paths
  - 0: HIT
  - 12 13: HIT

{main}
- branches
  - 00; OP: 00-29; line: 02-36 HIT; out1: EX  X 
- paths
  - 0: HIT
