--TEST--
Test for bug #1034: path coverage [3] (< PHP 7.3, !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.3; !opcache');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

include 'bug01034-003.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
?>
--EXPECTF--
Let's do some stuff!
caught
And do some more
Let's do some stuff!
caught
And do some more
Let's do some stuff!
CAUGHT IN MAIN
trycatch
- branches
  - 00; OP: 00-16; line: 06-13 HIT; out1: 17 HIT; out2: 22 HIT
  - 17; OP: 17-18; line: 14-14 HIT; out1: 19 HIT; out2: 29 HIT
  - 19; OP: 19-20; line: 15-15 HIT; out1: 21  X ; out2: 36 HIT
  - 21; OP: 21-21; line: 15-15  X ; out1: 43  X 
  - 22; OP: 22-28; line: 13-13 HIT; out1: EX  X 
  - 29; OP: 29-35; line: 14-14 HIT; out1: EX  X 
  - 36; OP: 36-42; line: 15-15 HIT; out1: EX  X 
  - 43; OP: 43-45; line: 17-17  X ; out1: 57  X 
  - 46; OP: 46-46; line: 18-18 HIT; out1: 47 HIT; out2: 50 HIT
  - 47; OP: 47-49; line: 19-19 HIT; out1: 57 HIT
  - 50; OP: 50-50; line: 20-20 HIT; out1: 51  X ; out2: 54 HIT
  - 51; OP: 51-53; line: 21-21  X ; out1: 57  X 
  - 54; OP: 54-54; line: 22-22 HIT; out1: 55 HIT; out2: EX  X 
  - 55; OP: 55-56; line: 23-26 HIT; out1: 57 HIT
  - 57; OP: 57-60; line: 26-27 HIT; out1: EX  X 
- paths
  - 0 17 19 21 43 57:  X 
  - 0 17 19 36: HIT
  - 0 17 29: HIT
  - 0 22: HIT
  - 46 47 57: HIT
  - 46 50 51 57:  X 
  - 46 50 54 55 57: HIT

{main}
- branches
  - 00; OP: 00-30; line: 02-32 HIT; out1: 34  X 
  - 31; OP: 31-31; line: 33-33 HIT; out1: 32 HIT; out2: EX  X 
  - 32; OP: 32-33; line: 34-37 HIT; out1: 34 HIT
  - 34; OP: 34-34; line: 37-37 HIT; out1: EX  X 
- paths
  - 0 34:  X 
  - 31 32 34: HIT
