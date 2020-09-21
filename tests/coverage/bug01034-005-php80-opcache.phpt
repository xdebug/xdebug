--TEST--
Test for bug #1034: path coverage [5] (>= PHP 8.0, opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0; opcache');
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
  - 00; OP: 00-04; line: 05-05 HIT; out1: EX  X 
  - 05; OP: 05-05; line: 07-07 HIT; out1: 06  X ; out2: 09 HIT
  - 06; OP: 06-08; line: 08-08  X ; out1: 20  X 
  - 09; OP: 09-09; line: 09-09 HIT; out1: 10  X ; out2: 13 HIT
  - 10; OP: 10-12; line: 10-10  X ; out1: 20  X 
  - 13; OP: 13-13; line: 11-11 HIT; out1: 14 HIT; out2: 17  X 
  - 14; OP: 14-16; line: 12-12 HIT; out1: 20 HIT
  - 17; OP: 17-17; line: 13-13  X ; out1: 18  X ; out2: EX  X 
  - 18; OP: 18-19; line: 14-16  X ; out1: 20  X 
  - 20; OP: 20-21; line: 16-16 HIT; out1: EX  X 
- paths
  - 0: HIT
  - 5 6 20:  X 
  - 5 9 10 20:  X 
  - 5 9 13 14 20: HIT
  - 5 9 13 17 18 20:  X 

{main}
- branches
  - 00; OP: 00-03; line: 17-19 HIT; out1: EX  X 
- paths
  - 0: HIT
