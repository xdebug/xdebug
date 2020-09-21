--TEST--
Test for bug #1210: Coverage of sending arguments to a method (>= PHP 7.4, < PHP 8.0, opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.4,< 8.0; opcache');
?>
--INI--
xdebug.mode=coverage
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
  - 00; OP: 00-05; line: 04-07  X ; out1: 06  X ; out2: 14  X 
  - 06; OP: 06-06; line: 07-07  X ; out1: 07  X ; out2: 14  X 
  - 07; OP: 07-13; line: 07-07  X ; out1: 06  X 
  - 14; OP: 14-19; line: 07-12  X ; out1: 20  X ; out2: 28  X 
  - 20; OP: 20-20; line: 12-12  X ; out1: 21  X ; out2: 28  X 
  - 21; OP: 21-27; line: 12-12  X ; out1: 20  X 
  - 28; OP: 28-33; line: 12-17  X ; out1: 34  X ; out2: 38  X 
  - 34; OP: 34-37; line: 18-21  X ; out1: 38  X 
  - 38; OP: 38-43; line: 21-23  X ; out1: EX  X 
- paths
  - 0 6 7 6 14 20 21 20 28 34 38:  X 
  - 0 6 7 6 14 20 21 20 28 38:  X 
  - 0 6 7 6 14 20 28 34 38:  X 
  - 0 6 7 6 14 20 28 38:  X 
  - 0 6 7 6 14 28 34 38:  X 
  - 0 6 7 6 14 28 38:  X 
  - 0 6 14 20 21 20 28 34 38:  X 
  - 0 6 14 20 21 20 28 38:  X 
  - 0 6 14 20 28 34 38:  X 
  - 0 6 14 20 28 38:  X 
  - 0 6 14 28 34 38:  X 
  - 0 6 14 28 38:  X 
  - 0 14 20 21 20 28 34 38:  X 
  - 0 14 20 21 20 28 38:  X 
  - 0 14 20 28 34 38:  X 
  - 0 14 20 28 38:  X 
  - 0 14 28 34 38:  X 
  - 0 14 28 38:  X 

{main}
- branches
  - 00; OP: 00-00; line: 27-27 HIT; out1: EX  X 
- paths
  - 0: HIT
