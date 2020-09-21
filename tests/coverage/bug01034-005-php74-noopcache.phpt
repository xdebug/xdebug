--TEST--
Test for bug #1034: path coverage [5] (>= PHP 7.4, < PHP 8.0, !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.4,< 8.0; !opcache');
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
  - 00; OP: 00-06; line: 02-05 HIT; out1: EX  X 
  - 10; OP: 10-10; line: 07-07 HIT; out1: 11  X ; out2: 14 HIT
  - 11; OP: 11-13; line: 08-08  X ; out1: 25  X 
  - 14; OP: 14-14; line: 09-09 HIT; out1: 15  X ; out2: 18 HIT
  - 15; OP: 15-17; line: 10-10  X ; out1: 25  X 
  - 18; OP: 18-18; line: 11-11 HIT; out1: 19 HIT; out2: 22  X 
  - 19; OP: 19-21; line: 12-12 HIT; out1: 25 HIT
  - 22; OP: 22-22; line: 13-13  X ; out1: 23  X ; out2: EX  X 
  - 23; OP: 23-24; line: 14-16  X ; out1: 25  X 
  - 25; OP: 25-26; line: 16-16 HIT; out1: EX  X 
- paths
  - 0: HIT
  - 10 11 25:  X 
  - 10 14 15 25:  X 
  - 10 14 18 19 25: HIT
  - 10 14 18 22 23 25:  X 

{main}
- branches
  - 00; OP: 00-03; line: 17-19 HIT; out1: EX  X 
- paths
  - 0: HIT
