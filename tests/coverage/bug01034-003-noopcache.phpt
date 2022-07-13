--TEST--
Test for bug #1034: path coverage [3] (!opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!opcache');
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
  - 00; OP: 00-13; line: 06-13 HIT; out1: 14 HIT; out2: 19 HIT
  - 14; OP: 14-15; line: 14-14 HIT; out1: 16 HIT; out2: 24 HIT
  - 16; OP: 16-17; line: 15-15 HIT; out1: 18  X ; out2: 29 HIT
  - 18; OP: 18-18; line: 15-15  X ; out1: 34  X
  - 19; OP: 19-23; line: 13-13 HIT; out1: EX  X
  - 24; OP: 24-28; line: 14-14 HIT; out1: EX  X
  - 29; OP: 29-33; line: 15-15 HIT; out1: EX  X
  - 34; OP: 34-36; line: 17-17  X ; out1: 48  X
  - 37; OP: 37-37; line: 18-18 HIT; out1: 38 HIT; out2: 41 HIT
  - 38; OP: 38-40; line: 19-19 HIT; out1: 48 HIT
  - 41; OP: 41-41; line: 20-20 HIT; out1: 42  X ; out2: 45 HIT
  - 42; OP: 42-44; line: 21-21  X ; out1: 48  X
  - 45; OP: 45-45; line: 22-22 HIT; out1: 46 HIT; out2: EX  X
  - 46; OP: 46-47; line: 23-26 HIT; out1: 48 HIT
  - 48; OP: 48-51; line: 26-27 HIT; out1: EX  X
- paths
  - 0 14 16 18 34 48:  X
  - 0 14 16 29: HIT
  - 0 14 24: HIT
  - 0 19: HIT
  - 37 38 48: HIT
  - 37 41 42 48:  X
  - 37 41 45 46 48: HIT

{main}
- branches
  - 00; OP: 00-13; line: 29-32 HIT; out1: 17  X
  - 14; OP: 14-14; line: 33-33 HIT; out1: 15 HIT; out2: EX  X
  - 15; OP: 15-16; line: 34-37 HIT; out1: 17 HIT
  - 17; OP: 17-17; line: 37-37 HIT; out1: EX  X
- paths
  - 0 17:  X
  - 14 15 17: HIT
