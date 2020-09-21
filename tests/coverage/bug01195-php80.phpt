--TEST--
Test for bug #1195: Segfault with code coverage and foreach (>= PHP 8.0, !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0; !opcache');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

include 'bug01195.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
?>
--EXPECTF--
foo
foo
foo
the end
fe
- branches
  - 00; OP: 00-02; line: 02-04 HIT; out1: 03 HIT; out2: 07  X 
  - 03; OP: 03-03; line: 04-04 HIT; out1: 04 HIT; out2: 07 HIT
  - 04; OP: 04-06; line: 06-04 HIT; out1: 03 HIT
  - 07; OP: 07-11; line: 04-09 HIT; out1: EX  X 
- paths
  - 0 3 4 3 7: HIT
  - 0 3 7:  X 
  - 0 7:  X 

{main}
- branches
  - 00; OP: 00-04; line: 11-13 HIT; out1: EX  X 
- paths
  - 0: HIT
