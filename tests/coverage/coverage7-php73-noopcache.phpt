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
  - 00; OP: 00-02; line: 12-15 HIT; out1: 03 HIT
  - 03; OP: 03-08; line: 15-16 HIT; out1: 09 HIT; out2: 03 HIT
  - 09; OP: 09-10; line: 17-17 HIT; out1: EX  X 
- paths
  - 0 3 9:  X 
  - 0 3 3 9: HIT

foo->ok
- branches
  - 00; OP: 00-04; line: 03-05 HIT; out1: 05 HIT; out2: 07  X 
  - 05; OP: 05-06; line: 05-05 HIT; out1: 07 HIT
  - 07; OP: 07-07; line: 05-05 HIT; out1: 08 HIT; out2: 11  X 
  - 08; OP: 08-10; line: 06-06 HIT; out1: 18 HIT
  - 11; OP: 11-13; line: 07-07  X ; out1: 14  X ; out2: 15  X 
  - 14; OP: 14-14; line: 07-07  X ; out1: 15  X 
  - 15; OP: 15-15; line: 07-07  X ; out1: 16  X ; out2: 18  X 
  - 16; OP: 16-17; line: 08-10  X ; out1: 18  X 
  - 18; OP: 18-19; line: 10-10 HIT; out1: EX  X 
- paths
  - 0 5 7 8 18: HIT
  - 0 5 7 11 14 15 16 18:  X 
  - 0 5 7 11 14 15 18:  X 
  - 0 5 7 11 15 16 18:  X 
  - 0 5 7 11 15 18:  X 
  - 0 7 8 18:  X 
  - 0 7 11 14 15 16 18:  X 
  - 0 7 11 14 15 18:  X 
  - 0 7 11 15 16 18:  X 
  - 0 7 11 15 18:  X 

foo->test_closure
- branches
  - 00; OP: 00-13; line: 19-26 HIT; out1: EX  X 
- paths
  - 0: HIT

{closure:%scoverage7.inc:21-23}
- branches
  - 00; OP: 00-07; line: 21-23 HIT
- paths
  - 0: HIT

{main}
- branches
  - 00; OP: 00-26; line: 02-35 HIT; out1: EX  X 
- paths
  - 0: HIT
