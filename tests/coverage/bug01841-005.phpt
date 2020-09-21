--TEST--
Test for bug #1841: 'match' keyword, with tmp var [5]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0');
?>
--INI--
xdebug.mode=coverage
xdebug.start_with_request=trigger
opcache.optimization_level=0
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

require dirname(__FILE__) . '/bug01841-005.inc';

$cc = xdebug_get_code_coverage();

dump_branch_coverage($cc);
xdebug_stop_code_coverage();
?>
--EXPECTF--
a
- branches
  - 00; OP: 00-05; line: 04-06 HIT; out1: 06 HIT; out2: 09  X 
  - 06; OP: 06-07; line: 08-08 HIT; out1: 08  X ; out2: 11 HIT
  - 08; OP: 08-08; line: 08-08  X ; out1: 13  X 
  - 09; OP: 09-10; line: 07-07  X ; out1: 15  X 
  - 11; OP: 11-12; line: 08-08 HIT; out1: 15 HIT
  - 13; OP: 13-14; line: 09-09  X ; out1: 15  X 
  - 15; OP: 15-20; line: 09-13 HIT
- paths
  - 0 6 8 13 15:  X 
  - 0 6 11 15: HIT
  - 0 9 15:  X 

{main}
- branches
  - 00; OP: 00-04; line: 15-16 HIT; out1: EX  X 
- paths
  - 0: HIT
