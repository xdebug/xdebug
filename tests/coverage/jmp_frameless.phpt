--TEST--
Test for JMP_FRAMELESS (>= PHP 8.4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.4');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK );

require dirname(__FILE__) . '/jmp_frameless.inc';

$cc = xdebug_get_code_coverage();
dump_branch_coverage($cc);
xdebug_stop_code_coverage();
?>
--EXPECTF--
Cool?
{main}
- branches
  - 00; OP: 00-%d; line: %d-04 HIT; out1: %d  X ; out2: %d HIT
  - %d; OP: %d-%d; line: 04-04  X ; out1: %d  X
  - %d; OP: %d-%d; line: 04-04 HIT; out1: %d HIT
  - %d; OP: %d-%d; line: 04-07 HIT; out1: EX  X
- paths
  - 0 %d %d:  X
  - 0 %d %d: HIT
