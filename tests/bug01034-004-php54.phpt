--TEST--
Test for bug #1034: path coverage [4] (< PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
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
  - 00; OP: 00-04; line: 12-14 HIT; out1: 05 HIT; out2: 08 HIT
  - 05; OP: 05-07; line: 15-16 HIT; out1: 08 HIT
  - 08; OP: 08-09; line: 17-17 HIT; out1: 10 HIT; out2: 13 HIT
  - 10; OP: 10-12; line: 18-19 HIT; out1: 13 HIT
  - 13; OP: 13-14; line: 20-20 HIT; out1: EX  X 
- paths
  - 0 5 8 10 13:  X 
  - 0 5 8 13: HIT
  - 0 8 10 13: HIT
  - 0 8 13:  X 

loopy
- branches
  - 00; OP: 00-02; line: 02-06 HIT; out1: 03 HIT
  - 03; OP: 03-07; line: 06-08 HIT; out1: 08 HIT; out2: 03  X 
  - 08; OP: 08-11; line: 09-10 HIT; out1: EX  X 
- paths
  - 0 3 8: HIT
  - 0 3 3 8:  X 

{main}
- branches
  - 00; OP: 00-22; line: 02-26 HIT; out1: EX  X 
- paths
  - 0: HIT
