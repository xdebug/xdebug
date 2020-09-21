--TEST--
Test for bug #1034: path coverage [1] (>= PHP 8.0, opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0; opcache');
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
  - 00; OP: 00-03; line: 10-12 HIT; out1: 04  X ; out2: 07 HIT
  - 04; OP: 04-06; line: 13-13  X ; out1: 11  X 
  - 07; OP: 07-10; line: 15-16 HIT; out1: EX  X 
  - 11; OP: 11-14; line: 18-19  X ; out1: EX  X 
- paths
  - 0 4 11:  X 
  - 0 7: HIT

loopy
- branches
  - 00; OP: 00-03; line: 02-04 HIT; out1: 09 HIT
  - 04; OP: 04-08; line: 05-04 HIT; out1: 09 HIT
  - 09; OP: 09-11; line: 04-04 HIT; out1: 12 HIT; out2: 04 HIT
  - 12; OP: 12-15; line: 07-08 HIT; out1: EX  X 
- paths
  - 0 9 12: HIT
  - 0 9 4 9 12: HIT

trycatch
- branches
  - 00; OP: 00-04; line: 24-24 HIT; out1: EX  X 
  - 05; OP: 05-05; line: 26-26 HIT; out1: 06 HIT; out2: EX  X 
  - 06; OP: 06-09; line: 27-29 HIT; out1: EX  X 
- paths
  - 0: HIT
  - 5 6: HIT

{main}
- branches
  - 00; OP: 00-15; line: 31-36 HIT; out1: EX  X 
- paths
  - 0: HIT
