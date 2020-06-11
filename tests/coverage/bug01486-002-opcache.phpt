--TEST--
Test for bug #1486: Crash on ZEND_SWITCH_LONG / ZEND_SWITCH_STRING (< PHP 7.3, opcache)
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
  - 00; OP: 00-03; line: 02-03 HIT; out1: 14  X ; out2: 21  X ; out3: 28  X ; out4: 35  X ; out5: 42  X ; out6: 48  X ; out7: 04 HIT
  - 04; OP: 04-05; line: 04-04 HIT; out1: 06 HIT; out2: 14  X 
  - 06; OP: 06-07; line: 05-05 HIT; out1: 08 HIT; out2: 21  X 
  - 08; OP: 08-09; line: 06-06 HIT; out1: 10  X ; out2: 28 HIT
  - 10; OP: 10-11; line: 07-07  X ; out1: 12  X ; out2: 35  X 
  - 12; OP: 12-13; line: 08-08  X ; out1: 48  X ; out2: 42  X 
  - 14; OP: 14-20; line: 04-10  X ; out1: EX  X 
  - 21; OP: 21-27; line: 05-10  X ; out1: EX  X 
  - 28; OP: 28-34; line: 06-10 HIT; out1: EX  X 
  - 35; OP: 35-41; line: 07-10  X ; out1: EX  X 
  - 42; OP: 42-47; line: 08-10  X ; out1: 48  X 
  - 48; OP: 48-48; line: 10-10  X ; out1: EX  X 
- paths
  - 0 14:  X 
  - 0 21:  X 
  - 0 28:  X 
  - 0 35:  X 
  - 0 42 48:  X 
  - 0 48:  X 
  - 0 4 6 8 10 12 48:  X 
  - 0 4 6 8 10 12 42 48:  X 
  - 0 4 6 8 10 35:  X 
  - 0 4 6 8 28: HIT
  - 0 4 6 21:  X 
  - 0 4 14:  X
