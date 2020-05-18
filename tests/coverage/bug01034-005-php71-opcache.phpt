--TEST--
Test for bug #1034: path coverage [5] (< PHP 7.3, opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.3; opcache');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

try {
	include 'bug01034-005.inc';
} catch (Exception $e) {
	/* eat */
}

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
?>
--EXPECTF--
caught
trycatch
- branches
  - 00; OP: 00-07; line: 02-05 HIT; out1: EX  X 
  - 08; OP: 08-08; line: 07-07 HIT; out1: 09  X ; out2: 12 HIT
  - 09; OP: 09-11; line: 08-08  X ; out1: 23  X 
  - 12; OP: 12-12; line: 09-09 HIT; out1: 13  X ; out2: 16 HIT
  - 13; OP: 13-15; line: 10-10  X ; out1: 23  X 
  - 16; OP: 16-16; line: 11-11 HIT; out1: 17 HIT; out2: 20  X 
  - 17; OP: 17-19; line: 12-12 HIT; out1: 23 HIT
  - 20; OP: 20-20; line: 13-13  X ; out1: 21  X ; out2: EX  X 
  - 21; OP: 21-22; line: 14-16  X ; out1: 23  X 
  - 23; OP: 23-24; line: 16-16 HIT; out1: EX  X 
- paths
  - 0: HIT
  - 8 9 23:  X 
  - 8 12 13 23:  X 
  - 8 12 16 17 23: HIT
  - 8 12 16 20 21 23:  X 

{main}
- branches
  - 00; OP: 00-06; line: 02-19 HIT; out1: EX  X 
- paths
  - 0: HIT
