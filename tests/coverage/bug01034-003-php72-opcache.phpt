--TEST--
Test for bug #1034: path coverage [3] (< PHP 7.3, opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.3; opcache');
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
  - 00; OP: 00-15; line: 06-13 HIT; out1: 16 HIT; out2: 20 HIT
  - 16; OP: 16-17; line: 14-14 HIT; out1: 18 HIT; out2: 27 HIT
  - 18; OP: 18-19; line: 15-15 HIT; out1: 41  X ; out2: 34 HIT
  - 20; OP: 20-26; line: 13-13 HIT; out1: EX  X 
  - 27; OP: 27-33; line: 14-14 HIT; out1: EX  X 
  - 34; OP: 34-40; line: 15-15 HIT; out1: EX  X 
  - 41; OP: 41-43; line: 17-17  X ; out1: 55  X 
  - 44; OP: 44-44; line: 18-18 HIT; out1: 45 HIT; out2: 48 HIT
  - 45; OP: 45-47; line: 19-19 HIT; out1: 55 HIT
  - 48; OP: 48-48; line: 20-20 HIT; out1: 49  X ; out2: 52 HIT
  - 49; OP: 49-51; line: 21-21  X ; out1: 55  X 
  - 52; OP: 52-52; line: 22-22 HIT; out1: 53 HIT; out2: EX  X 
  - 53; OP: 53-54; line: 23-26 HIT; out1: 55 HIT
  - 55; OP: 55-58; line: 26-27 HIT; out1: EX  X 
- paths
  - 0 16 18 41 55:  X 
  - 0 16 18 34: HIT
  - 0 16 27: HIT
  - 0 20: HIT
  - 44 45 55: HIT
  - 44 48 49 55:  X 
  - 44 48 52 53 55: HIT

{main}
- branches
  - 00; OP: 00-28; line: 02-37 HIT; out1: EX  X 
  - 29; OP: 29-29; line: 33-33 HIT; out1: 30 HIT; out2: EX  X 
  - 30; OP: 30-32; line: 34-37 HIT; out1: EX  X 
- paths
  - 0: HIT
  - 29 30: HIT
