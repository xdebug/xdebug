--TEST--
Test for bug #1034: path coverage [3] (>= PHP 7.4, < PHP 8.0, !opcache)
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
  - 00; OP: 00-14; line: 06-13 HIT; out1: 15 HIT; out2: 20 HIT
  - 15; OP: 15-16; line: 14-14 HIT; out1: 17 HIT; out2: 25 HIT
  - 17; OP: 17-18; line: 15-15 HIT; out1: 19  X ; out2: 30 HIT
  - 19; OP: 19-19; line: 15-15  X ; out1: 35  X 
  - 20; OP: 20-24; line: 13-13 HIT; out1: EX  X 
  - 25; OP: 25-29; line: 14-14 HIT; out1: EX  X 
  - 30; OP: 30-34; line: 15-15 HIT; out1: EX  X 
  - 35; OP: 35-37; line: 17-17  X ; out1: 49  X 
  - 38; OP: 38-38; line: 18-18 HIT; out1: 39 HIT; out2: 42 HIT
  - 39; OP: 39-41; line: 19-19 HIT; out1: 49 HIT
  - 42; OP: 42-42; line: 20-20 HIT; out1: 43  X ; out2: 46 HIT
  - 43; OP: 43-45; line: 21-21  X ; out1: 49  X 
  - 46; OP: 46-46; line: 22-22 HIT; out1: 47 HIT; out2: EX  X 
  - 47; OP: 47-48; line: 23-26 HIT; out1: 49 HIT
  - 49; OP: 49-52; line: 26-27 HIT; out1: EX  X 
- paths
  - 0 15 17 19 35 49:  X 
  - 0 15 17 30: HIT
  - 0 15 25: HIT
  - 0 20: HIT
  - 38 39 49: HIT
  - 38 42 43 49:  X 
  - 38 42 46 47 49: HIT

{main}
- branches
  - 00; OP: 00-13; line: 29-32 HIT; out1: 17  X 
  - 14; OP: 14-14; line: 33-33 HIT; out1: 15 HIT; out2: EX  X 
  - 15; OP: 15-16; line: 34-37 HIT; out1: 17 HIT
  - 17; OP: 17-17; line: 37-37 HIT; out1: EX  X 
- paths
  - 0 17:  X 
  - 14 15 17: HIT
