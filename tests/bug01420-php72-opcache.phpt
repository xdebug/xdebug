--TEST--
Test for bug #1420: handle path/branch converage for switch with jump table (>= PHP 7.2, opcache)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.2", '>=')) echo "skip >= PHP 7.2 needed\n"; ?>
<?php if (!extension_loaded('zend opcache')) echo "skip opcache required\n"; ?>
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

include 'bug01420.inc';

xdebug_stop_code_coverage(false);
$cc = xdebug_get_code_coverage();
dump_branch_coverage($cc);
?>
--EXPECTF--
baz
{main}
- branches
  - 00; OP: 00-03; line: 02-03 HIT; out1: 10  X ; out2: 14 HIT; out3: 18  X ; out4: 22  X ; out5: 04  X 
  - 04; OP: 04-05; line: 04-04  X ; out1: 06  X ; out2: 10  X 
  - 06; OP: 06-07; line: 07-07  X ; out1: 08  X ; out2: 14  X 
  - 08; OP: 08-09; line: 10-10  X ; out1: 22  X ; out2: 18  X 
  - 10; OP: 10-13; line: 05-18  X ; out1: EX  X 
  - 14; OP: 14-17; line: 08-18 HIT; out1: EX  X 
  - 18; OP: 18-21; line: 11-18  X ; out1: EX  X 
  - 22; OP: 22-25; line: 14-18  X ; out1: EX  X 
- paths
  - 0 10:  X 
  - 0 14: HIT
  - 0 18:  X 
  - 0 22:  X 
  - 0 4 6 8 22:  X 
  - 0 4 6 8 18:  X 
  - 0 4 6 14:  X 
  - 0 4 10:  X
