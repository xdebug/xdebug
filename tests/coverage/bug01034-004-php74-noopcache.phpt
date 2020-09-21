--TEST--
Test for bug #1034: path coverage [4] (>= PHP 7.4, < PHP 8.0, !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.4,< 8.0; !opcache');
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
  - 00; OP: 00-04; line: 12-14 HIT; out1: 05 HIT; out2: 07 HIT
  - 05; OP: 05-06; line: 15-17 HIT; out1: 07 HIT
  - 07; OP: 07-08; line: 17-17 HIT; out1: 09 HIT; out2: 11 HIT
  - 09; OP: 09-10; line: 18-20 HIT; out1: 11 HIT
  - 11; OP: 11-12; line: 20-20 HIT; out1: EX  X 
- paths
  - 0 5 7 9 11:  X 
  - 0 5 7 11: HIT
  - 0 7 9 11: HIT
  - 0 7 11:  X 

loopy
- branches
  - 00; OP: 00-02; line: 02-06 HIT; out1: 03 HIT
  - 03; OP: 03-08; line: 06-08 HIT; out1: 09 HIT; out2: 03  X 
  - 09; OP: 09-12; line: 09-10 HIT; out1: EX  X 
- paths
  - 0 3 9: HIT
  - 0 3 3 9:  X 

{main}
- branches
  - 00; OP: 00-14; line: 22-26 HIT; out1: EX  X 
- paths
  - 0: HIT
