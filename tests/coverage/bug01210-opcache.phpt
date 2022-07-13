--TEST--
Test for bug #1210: Coverage of sending arguments to a method (opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('opcache');
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
  - 00; OP: 00-04; line: 06-07  X ; out1: 05  X ; out2: 13  X
  - 05; OP: 05-05; line: 07-07  X ; out1: 06  X ; out2: 13  X
  - 06; OP: 06-12; line: 07-07  X ; out1: 05  X
  - 13; OP: 13-18; line: 07-12  X ; out1: 19  X ; out2: 27  X
  - 19; OP: 19-19; line: 12-12  X ; out1: 20  X ; out2: 27  X
  - 20; OP: 20-26; line: 12-12  X ; out1: 19  X
  - 27; OP: 27-32; line: 12-17  X ; out1: 33  X ; out2: 37  X
  - 33; OP: 33-36; line: 18-21  X ; out1: 37  X
  - 37; OP: 37-42; line: 21-23  X ; out1: EX  X
- paths
  - 0 5 6 5 13 19 20 19 27 33 37:  X
  - 0 5 6 5 13 19 20 19 27 37:  X
  - 0 5 6 5 13 19 27 33 37:  X
  - 0 5 6 5 13 19 27 37:  X
  - 0 5 6 5 13 27 33 37:  X
  - 0 5 6 5 13 27 37:  X
  - 0 5 13 19 20 19 27 33 37:  X
  - 0 5 13 19 20 19 27 37:  X
  - 0 5 13 19 27 33 37:  X
  - 0 5 13 19 27 37:  X
  - 0 5 13 27 33 37:  X
  - 0 5 13 27 37:  X
  - 0 13 19 20 19 27 33 37:  X
  - 0 13 19 20 19 27 37:  X
  - 0 13 19 27 33 37:  X
  - 0 13 19 27 37:  X
  - 0 13 27 33 37:  X
  - 0 13 27 37:  X

{main}
- branches
  - 00; OP: 00-00; line: 27-27 HIT; out1: EX  X
- paths
  - 0: HIT
