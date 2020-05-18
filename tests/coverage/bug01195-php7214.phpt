--TEST--
Test for bug #1195: Segfault with code coverage and foreach (> PHP 7.2.13, < PHP 7.4, !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP > 7.2.13,!= 7.3.0,< 7.4; !opcache');
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
  - 00; OP: 00-03; line: 02-04 HIT; out1: 04 HIT; out2: 08  X 
  - 04; OP: 04-04; line: 04-04 HIT; out1: 05 HIT; out2: 08 HIT
  - 05; OP: 05-07; line: 06-04 HIT; out1: 04 HIT
  - 08; OP: 08-12; line: 04-09 HIT; out1: EX  X 
- paths
  - 0 4 5 4 8: HIT
  - 0 4 8:  X 
  - 0 8:  X 

{main}
- branches
  - 00; OP: 00-08; line: 02-13 HIT; out1: EX  X 
- paths
  - 0: HIT
