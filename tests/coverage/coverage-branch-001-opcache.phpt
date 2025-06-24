--TEST--
Test with Code Coverage with path and branch checking with two identical functions (opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('opcache');
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
  - 00; OP: 00-02; line: 02-04  X ; out1: 03  X ; out2: 05  X
  - 03; OP: 03-04; line: 05-05  X ; out1: EX  X
  - 05; OP: 05-06; line: 07-07  X ; out1: EX  X
- paths
  - 0 3:  X
  - 0 5:  X

test_b
- branches
  - 00; OP: 00-02; line: 12-14 HIT; out1: 03 HIT; out2: 05  X
  - 03; OP: 03-04; line: 15-15 HIT; out1: EX  X
  - 05; OP: 05-06; line: 17-17  X ; out1: EX  X
- paths
  - 0 3: HIT
  - 0 5:  X

{main}
- branches
  - 00; OP: 00-00; line: 22-22 HIT; out1: EX  X
- paths
  - 0: HIT
