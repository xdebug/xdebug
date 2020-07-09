--TEST--
Test with Code Coverage with path and branch checking (< PHP 7.4, !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.4; !opcache');
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
  - 00; OP: 00-02; line: 12-15 HIT; out1: 03 HIT
  - 03; OP: 03-08; line: 15-16 HIT; out1: 09 HIT; out2: 03 HIT
  - 09; OP: 09-10; line: 17-17 HIT; out1: EX  X 
- paths
  - 0 3 9: HIT
  - 0 3 3 9: HIT

ok
- branches
  - 00; OP: 00-04; line: 02-04 HIT; out1: 05 HIT; out2: 07 HIT
  - 05; OP: 05-06; line: 04-04 HIT; out1: 07 HIT
  - 07; OP: 07-07; line: 04-04 HIT; out1: 08 HIT; out2: 10 HIT
  - 08; OP: 08-09; line: 05-07 HIT; out1: 10 HIT
  - 10; OP: 10-12; line: 07-07 HIT; out1: 13 HIT; out2: 14 HIT
  - 13; OP: 13-13; line: 07-07 HIT; out1: 14 HIT
  - 14; OP: 14-14; line: 07-07 HIT; out1: 15 HIT; out2: 17 HIT
  - 15; OP: 15-16; line: 08-10 HIT; out1: 17 HIT
  - 17; OP: 17-18; line: 10-10 HIT; out1: EX  X 
- paths
  - 0 5 7 8 10 13 14 15 17:  X 
  - 0 5 7 8 10 13 14 17:  X 
  - 0 5 7 8 10 14 15 17:  X 
  - 0 5 7 8 10 14 17: HIT
  - 0 5 7 10 13 14 15 17:  X 
  - 0 5 7 10 13 14 17:  X 
  - 0 5 7 10 14 15 17:  X 
  - 0 5 7 10 14 17: HIT
  - 0 7 8 10 13 14 15 17:  X 
  - 0 7 8 10 13 14 17:  X 
  - 0 7 8 10 14 15 17:  X 
  - 0 7 8 10 14 17:  X 
  - 0 7 10 13 14 15 17: HIT
  - 0 7 10 13 14 17: HIT
  - 0 7 10 14 15 17:  X 
  - 0 7 10 14 17:  X 

{main}
- branches
  - 00; OP: 00-44; line: 02-26 HIT; out1: EX  X 
- paths
  - 0: HIT
