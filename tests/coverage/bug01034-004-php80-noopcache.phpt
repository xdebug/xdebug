--TEST--
Test for bug #1034: path coverage [4] (>= PHP 8.0, !opcache)
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

include 'bug01034-004.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
?>
--EXPECTF--
0 
A HIT
B HIT
ifthenelse
- branches
  - 00; OP: 00-03; line: 12-14 HIT; out1: 04 HIT; out2: 06 HIT
  - 04; OP: 04-05; line: 15-17 HIT; out1: 06 HIT
  - 06; OP: 06-07; line: 17-17 HIT; out1: 08 HIT; out2: 10 HIT
  - 08; OP: 08-09; line: 18-20 HIT; out1: 10 HIT
  - 10; OP: 10-11; line: 20-20 HIT; out1: EX  X 
- paths
  - 0 4 6 8 10:  X 
  - 0 4 6 10: HIT
  - 0 6 8 10: HIT
  - 0 6 10:  X 

loopy
- branches
  - 00; OP: 00-01; line: 02-06 HIT; out1: 02 HIT
  - 02; OP: 02-07; line: 06-08 HIT; out1: 08 HIT; out2: 02  X 
  - 08; OP: 08-11; line: 09-10 HIT; out1: EX  X 
- paths
  - 0 2 8: HIT
  - 0 2 2 8:  X 

{main}
- branches
  - 00; OP: 00-14; line: 22-26 HIT; out1: EX  X 
- paths
  - 0: HIT
