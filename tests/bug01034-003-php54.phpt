--TEST--
Test for bug #1034: path coverage [3] (< PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
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
  - 00; OP: 00-16; line: 06-13 HIT; out1: 17 HIT; out2: 26 HIT
  - 17; OP: 17-24; line: 13-13 HIT; out1: EX  X 
  - 26; OP: 26-28; line: 14-14 HIT; out1: 29 HIT; out2: 38 HIT
  - 29; OP: 29-36; line: 14-14 HIT; out1: EX  X 
  - 38; OP: 38-40; line: 15-15 HIT; out1: 41 HIT; out2: 50  X 
  - 41; OP: 41-48; line: 15-15 HIT; out1: EX  X 
  - 50; OP: 50-52; line: 17-18  X ; out1: 64  X 
  - 53; OP: 53-53; line: 18-18 HIT; out1: 54 HIT; out2: 57 HIT
  - 54; OP: 54-56; line: 19-20 HIT; out1: 64 HIT
  - 57; OP: 57-57; line: 20-20 HIT; out1: 58  X ; out2: 61 HIT
  - 58; OP: 58-60; line: 21-22  X ; out1: 64  X 
  - 61; OP: 61-61; line: 22-22 HIT; out1: 62 HIT; out2: EX  X 
  - 62; OP: 62-63; line: 23-26 HIT; out1: 64 HIT
  - 64; OP: 64-67; line: 26-27 HIT; out1: EX  X 
- paths
  - 0 17: HIT
  - 0 26 29: HIT
  - 0 26 38 41: HIT
  - 0 26 38 50 64:  X 
  - 53 54 64: HIT
  - 53 57 58 64:  X 
  - 53 57 61 62 64: HIT

{main}
- branches
  - 00; OP: 00-27; line: 02-33 HIT; out1: 31  X 
  - 28; OP: 28-28; line: 33-33 HIT; out1: 29 HIT; out2: EX  X 
  - 29; OP: 29-30; line: 34-37 HIT; out1: 31 HIT
  - 31; OP: 31-32; line: 37-37 HIT; out1: EX  X 
- paths
  - 0 31:  X 
  - 28 29 31: HIT
