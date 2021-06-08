--TEST--
Test with Code Coverage with path and branch checking with two identical functions (!opcache)
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

include 'coverage-branch-001.inc';
test_b(true);

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
?>
--EXPECT--
test_a
- branches
  - 00; OP: 00-03; line: 02-04  X ; out1: 04  X ; out2: 07  X 
  - 04; OP: 04-05; line: 05-05  X ; out1: EX  X 
  - 07; OP: 07-12; line: 07-10  X 
- paths
  - 0 4:  X 
  - 0 7:  X 

test_b
- branches
  - 00; OP: 00-03; line: 12-14 HIT; out1: 04 HIT; out2: 07  X 
  - 04; OP: 04-05; line: 15-15 HIT; out1: EX  X 
  - 07; OP: 07-12; line: 17-20  X 
- paths
  - 0 4: HIT
  - 0 7:  X 

{main}
- branches
  - 00; OP: 00-04; line: 02-22 HIT; out1: EX  X 
- paths
  - 0: HIT
