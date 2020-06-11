--TEST--
Test for bug #1034: path coverage [5] (< PHP 7.3, !opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.3; !opcache');
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
  - 00; OP: 00-08; line: 02-05 HIT; out1: EX  X 
  - 12; OP: 12-12; line: 07-07 HIT; out1: 13  X ; out2: 16 HIT
  - 13; OP: 13-15; line: 08-08  X ; out1: 27  X 
  - 16; OP: 16-16; line: 09-09 HIT; out1: 17  X ; out2: 20 HIT
  - 17; OP: 17-19; line: 10-10  X ; out1: 27  X 
  - 20; OP: 20-20; line: 11-11 HIT; out1: 21 HIT; out2: 24  X 
  - 21; OP: 21-23; line: 12-12 HIT; out1: 27 HIT
  - 24; OP: 24-24; line: 13-13  X ; out1: 25  X ; out2: EX  X 
  - 25; OP: 25-26; line: 14-16  X ; out1: 27  X 
  - 27; OP: 27-28; line: 16-16 HIT; out1: EX  X 
- paths
  - 0: HIT
  - 12 13 27:  X 
  - 12 16 17 27:  X 
  - 12 16 20 21 27: HIT
  - 12 16 20 24 25 27:  X 

{main}
- branches
  - 00; OP: 00-07; line: 02-19 HIT; out1: EX  X 
- paths
  - 0: HIT
