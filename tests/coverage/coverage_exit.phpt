--TEST--
Dummy
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

include 'coverage_exit.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
?>
--EXPECT--
exit_test
- branches
  - 00; OP: 00-06; line: 04-04 HIT; out1: 07  X ; out2: 12 HIT
  - 07; OP: 07-08; line: 05-05  X ; out1: EX  X
  - 12; OP: 12-13; line: 09-09 HIT; out1: EX  X
- paths
  - 0 7:  X
  - 0 12: HIT

{main}
- branches
  - 00; OP: 00-03; line: 11-13 HIT; out1: EX  X
- paths
  - 0: HIT
