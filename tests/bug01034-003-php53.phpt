--TEST--
Test for bug #1034: path coverage [3]
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.3", '>=')) echo "skip PHP 5.3 needed\n"; ?>
<?php if (!version_compare(phpversion(), "5.4", '<')) echo "skip PHP 5.3 needed\n"; ?>
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
  - 50; OP: 50-52; line: 17-18  X ; out1: 67  X 
  - 54; OP: 54-54; line: 18-18 HIT; out1: 55 HIT; out2: 58 HIT
  - 55; OP: 55-57; line: 19-20 HIT; out1: 67 HIT
  - 58; OP: 58-58; line: 20-20 HIT; out1: 59 HIT
  - 59; OP: 59-59; line: 20-20 HIT; out1: 60  X ; out2: 63 HIT
  - 60; OP: 60-62; line: 21-22  X ; out1: 67  X 
  - 63; OP: 63-63; line: 22-22 HIT; out1: 64 HIT
  - 64; OP: 64-64; line: 22-22 HIT; out1: 65 HIT; out2: EX  X 
  - 65; OP: 65-66; line: 23-26 HIT; out1: 67 HIT
  - 67; OP: 67-70; line: 26-27 HIT; out1: EX  X 
- paths
  - 0 17: HIT
  - 0 26 29: HIT
  - 0 26 38 41: HIT
  - 0 26 38 50 67:  X 
  - 54 55 67: HIT
  - 54 58 59 60 67:  X 
  - 54 58 59 63 64 65 67: HIT

{main}
- branches
  - 00; OP: 00-27; line: 02-33 HIT; out1: 32  X 
  - 29; OP: 29-29; line: 33-33 HIT; out1: 30 HIT; out2: EX  X 
  - 30; OP: 30-31; line: 34-37 HIT; out1: 32 HIT
  - 32; OP: 32-33; line: 37-37 HIT; out1: EX  X 
- paths
  - 0 32:  X 
  - 29 30 32: HIT
