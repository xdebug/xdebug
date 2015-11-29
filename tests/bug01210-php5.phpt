--TEST--
Test for bug #1210: Coverage of sending arguments to a method (< PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

include 'bug01210.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
?>
--EXPECT--
foo->getLoader
- branches
  - 00; OP: 00-08; line: 04-07  X ; out1: 09  X ; out2: 21  X 
  - 09; OP: 09-09; line: 07-07  X ; out1: 10  X ; out2: 21  X 
  - 10; OP: 10-20; line: 07-09  X ; out1: 09  X 
  - 21; OP: 21-29; line: 09-12  X ; out1: 30  X ; out2: 42  X 
  - 30; OP: 30-30; line: 12-12  X ; out1: 31  X ; out2: 42  X 
  - 31; OP: 31-41; line: 12-14  X ; out1: 30  X 
  - 42; OP: 42-50; line: 14-17  X ; out1: 51  X ; out2: 58  X 
  - 51; OP: 51-57; line: 18-19  X ; out1: 58  X 
  - 58; OP: 58-67; line: 21-24  X 
- paths
  - 0 9 10 9 21 30 31 30 42 51 58:  X 
  - 0 9 10 9 21 30 31 30 42 58:  X 
  - 0 9 10 9 21 30 42 51 58:  X 
  - 0 9 10 9 21 30 42 58:  X 
  - 0 9 10 9 21 42 51 58:  X 
  - 0 9 10 9 21 42 58:  X 
  - 0 9 21 30 31 30 42 51 58:  X 
  - 0 9 21 30 31 30 42 58:  X 
  - 0 9 21 30 42 51 58:  X 
  - 0 9 21 30 42 58:  X 
  - 0 9 21 42 51 58:  X 
  - 0 9 21 42 58:  X 
  - 0 21 30 31 30 42 51 58:  X 
  - 0 21 30 31 30 42 58:  X 
  - 0 21 30 42 51 58:  X 
  - 0 21 30 42 58:  X 
  - 0 21 42 51 58:  X 
  - 0 21 42 58:  X 

{main}
- branches
  - 00; OP: 00-02; line: 03-27 HIT; out1: EX  X 
- paths
  - 0: HIT
