--TEST--
Test with Code Coverage with path and branch checking (!opcache, <= PHP 8.2.8)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP <= 8.2.8; !opcache');
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

include 'coverage7.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
?>
--EXPECTF--
A NOT B
2
1
foo->loop_test
- branches
  - 00; OP: 00-01; line: 12-15 HIT; out1: 02 HIT
  - 02; OP: 02-07; line: 15-16 HIT; out1: 08 HIT; out2: 02 HIT
  - 08; OP: 08-09; line: 17-17 HIT; out1: EX  X
- paths
  - 0 2 8:  X
  - 0 2 2 8: HIT

foo->ok
- branches
  - 00; OP: 00-03; line: 03-05 HIT; out1: 04 HIT; out2: 06  X
  - 04; OP: 04-05; line: 05-05 HIT; out1: 06 HIT
  - 06; OP: 06-06; line: 05-05 HIT; out1: 07 HIT; out2: 10  X
  - 07; OP: 07-09; line: 06-06 HIT; out1: 17 HIT
  - 10; OP: 10-12; line: 07-07  X ; out1: 13  X ; out2: 14  X
  - 13; OP: 13-13; line: 07-07  X ; out1: 14  X
  - 14; OP: 14-14; line: 07-07  X ; out1: 15  X ; out2: 17  X
  - 15; OP: 15-16; line: 08-10  X ; out1: 17  X
  - 17; OP: 17-18; line: 10-10 HIT; out1: EX  X
- paths
  - 0 4 6 7 17: HIT
  - 0 4 6 10 13 14 15 17:  X
  - 0 4 6 10 13 14 17:  X
  - 0 4 6 10 14 15 17:  X
  - 0 4 6 10 14 17:  X
  - 0 6 7 17:  X
  - 0 6 10 13 14 15 17:  X
  - 0 6 10 13 14 17:  X
  - 0 6 10 14 15 17:  X
  - 0 6 10 14 17:  X

foo->test_closure
- branches
  - 00; OP: 00-10; line: 21-26 HIT; out1: EX  X
- paths
  - 0: HIT

{closure:%scoverage7.inc:21-23}
- branches
  - 00; OP: 00-06; line: 21-23 HIT
- paths
  - 0: HIT

{main}
- branches
  - 00; OP: 00-16; line: 29-35 HIT; out1: EX  X
- paths
  - 0: HIT
