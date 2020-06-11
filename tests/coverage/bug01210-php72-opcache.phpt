--TEST--
Test for bug #1210: Coverage of sending arguments to a method (<= PHP 7.2.13, opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP <= 7.2.13; opcache');
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
  - 00; OP: 00-07; line: 04-07  X ; out1: 08  X ; out2: 18  X 
  - 08; OP: 08-08; line: 07-07  X ; out1: 09  X ; out2: 18  X 
  - 09; OP: 09-17; line: 07-08  X ; out1: 08  X 
  - 18; OP: 18-25; line: 08-12  X ; out1: 26  X ; out2: 36  X 
  - 26; OP: 26-26; line: 12-12  X ; out1: 27  X ; out2: 36  X 
  - 27; OP: 27-35; line: 12-13  X ; out1: 26  X 
  - 36; OP: 36-43; line: 13-17  X ; out1: 44  X ; out2: 50  X 
  - 44; OP: 44-49; line: 18-21  X ; out1: 50  X 
  - 50; OP: 50-57; line: 21-23  X ; out1: EX  X 
- paths
  - 0 8 9 8 18 26 27 26 36 44 50:  X 
  - 0 8 9 8 18 26 27 26 36 50:  X 
  - 0 8 9 8 18 26 36 44 50:  X 
  - 0 8 9 8 18 26 36 50:  X 
  - 0 8 9 8 18 36 44 50:  X 
  - 0 8 9 8 18 36 50:  X 
  - 0 8 18 26 27 26 36 44 50:  X 
  - 0 8 18 26 27 26 36 50:  X 
  - 0 8 18 26 36 44 50:  X 
  - 0 8 18 26 36 50:  X 
  - 0 8 18 36 44 50:  X 
  - 0 8 18 36 50:  X 
  - 0 18 26 27 26 36 44 50:  X 
  - 0 18 26 27 26 36 50:  X 
  - 0 18 26 36 44 50:  X 
  - 0 18 26 36 50:  X 
  - 0 18 36 44 50:  X 
  - 0 18 36 50:  X 

{main}
- branches
  - 00; OP: 00-01; line: 02-27 HIT; out1: EX  X 
- paths
  - 0: HIT
