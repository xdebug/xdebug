--TEST--
Test for bug #1486: Crash on ZEND_SWITCH_LONG / ZEND_SWITCH_STRING (!opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!opcache');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
include 'dump-branch-coverage.inc';

$foo = [3, 1];
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK );
include dirname( __FILE__ ) . '/bug01486-002.inc';
$cc = xdebug_get_code_coverage();
dump_branch_coverage($cc);
xdebug_stop_code_coverage();
?>
--EXPECTF--
foo 3
{main}
- branches
  - 00; OP: 00-03; line: 02-03 HIT; out1: 15  X ; out2: 22  X ; out3: 29  X ; out4: 36  X ; out5: 43  X ; out6: 50  X ; out7: 04 HIT
  - 04; OP: 04-05; line: 04-04 HIT; out1: 06 HIT; out2: 15  X
  - 06; OP: 06-07; line: 05-05 HIT; out1: 08 HIT; out2: 22  X
  - 08; OP: 08-09; line: 06-06 HIT; out1: 10  X ; out2: 29 HIT
  - 10; OP: 10-11; line: 07-07  X ; out1: 12  X ; out2: 36  X
  - 12; OP: 12-13; line: 08-08  X ; out1: 14  X ; out2: 43  X
  - 14; OP: 14-14; line: 08-08  X ; out1: 50  X
  - 15; OP: 15-21; line: 04-04  X ; out1: 50  X
  - 22; OP: 22-28; line: 05-05  X ; out1: 50  X
  - 29; OP: 29-35; line: 06-06 HIT; out1: 50 HIT
  - 36; OP: 36-42; line: 07-07  X ; out1: 50  X
  - 43; OP: 43-49; line: 08-08  X ; out1: 50  X
  - 50; OP: 50-50; line: 10-10 HIT; out1: EX  X
- paths
  - 0 15 50:  X
  - 0 22 50:  X
  - 0 29 50:  X
  - 0 36 50:  X
  - 0 43 50:  X
  - 0 50:  X
  - 0 4 6 8 10 12 14 50:  X
  - 0 4 6 8 10 12 43 50:  X
  - 0 4 6 8 10 36 50:  X
  - 0 4 6 8 29 50: HIT
  - 0 4 6 22 50:  X
  - 0 4 15 50:  X
