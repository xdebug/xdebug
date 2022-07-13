--TEST--
Test with Code Coverage with path and branch checking (!opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!opcache');
?>
--INI--
xdebug.mode=coverage
xdebug.trace_options=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.dump_globals=0
xdebug.trace_format=0
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

include 'coverage6.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
?>
--EXPECT--
A NOT B
B NOT A
1
2
1
loop_test
- branches
  - 00; OP: 00-01; line: 12-15 HIT; out1: 02 HIT
  - 02; OP: 02-07; line: 15-16 HIT; out1: 08 HIT; out2: 02 HIT
  - 08; OP: 08-09; line: 17-17 HIT; out1: EX  X
- paths
  - 0 2 8: HIT
  - 0 2 2 8: HIT

ok
- branches
  - 00; OP: 00-03; line: 02-04 HIT; out1: 04 HIT; out2: 06 HIT
  - 04; OP: 04-05; line: 04-04 HIT; out1: 06 HIT
  - 06; OP: 06-06; line: 04-04 HIT; out1: 07 HIT; out2: 09 HIT
  - 07; OP: 07-08; line: 05-07 HIT; out1: 09 HIT
  - 09; OP: 09-11; line: 07-07 HIT; out1: 12 HIT; out2: 13 HIT
  - 12; OP: 12-12; line: 07-07 HIT; out1: 13 HIT
  - 13; OP: 13-13; line: 07-07 HIT; out1: 14 HIT; out2: 16 HIT
  - 14; OP: 14-15; line: 08-10 HIT; out1: 16 HIT
  - 16; OP: 16-17; line: 10-10 HIT; out1: EX  X
- paths
  - 0 4 6 7 9 12 13 14 16:  X
  - 0 4 6 7 9 12 13 16:  X
  - 0 4 6 7 9 13 14 16:  X
  - 0 4 6 7 9 13 16: HIT
  - 0 4 6 9 12 13 14 16:  X
  - 0 4 6 9 12 13 16:  X
  - 0 4 6 9 13 14 16:  X
  - 0 4 6 9 13 16: HIT
  - 0 6 7 9 12 13 14 16:  X
  - 0 6 7 9 12 13 16:  X
  - 0 6 7 9 13 14 16:  X
  - 0 6 7 9 13 16:  X
  - 0 6 9 12 13 14 16: HIT
  - 0 6 9 12 13 16: HIT
  - 0 6 9 13 14 16:  X
  - 0 6 9 13 16:  X

{main}
- branches
  - 00; OP: 00-28; line: 19-26 HIT; out1: EX  X
- paths
  - 0: HIT
