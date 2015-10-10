--TEST--
Test for bug #1034: path coverage [2] (< PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
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
  - 00; OP: 00-01; line: 02-02 HIT; out1: 02 HIT
  - 02; OP: 02-04; line: 02-02 HIT; out1: 12 HIT; out2: 08 HIT
  - 05; OP: 05-07; line: 02-02 HIT; out1: 02 HIT
  - 08; OP: 08-11; line: 03-04 HIT; out1: 05 HIT
  - 12; OP: 12-15; line: 05-07 HIT; out1: EX  X 
- paths
  - 0 2 12:  X 
  - 0 2 8 5 2 12: HIT
