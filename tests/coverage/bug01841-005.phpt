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
  - 00; OP: 00-06; line: 02-06 HIT; out1: 07 HIT; out2: 10  X 
  - 07; OP: 07-08; line: 08-08 HIT; out1: 09  X ; out2: 12 HIT
  - 09; OP: 09-09; line: 08-08  X ; out1: 14  X 
  - 10; OP: 10-11; line: 07-07  X ; out1: 16  X 
  - 12; OP: 12-13; line: 08-08 HIT; out1: 16 HIT
  - 14; OP: 14-15; line: 09-09  X ; out1: 16  X 
  - 16; OP: 16-21; line: 09-13 HIT
- paths
  - 0 7 9 14 16:  X 
  - 0 7 12 16: HIT
  - 0 10 16:  X 

{main}
- branches
  - 00; OP: 00-04; line: 15-16 HIT; out1: EX  X 
- paths
  - 0: HIT
