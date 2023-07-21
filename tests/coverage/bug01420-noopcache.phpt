--TEST--
Test for bug #1420: handle path/branch converage for switch with jump table (!opcache)
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
  - 00; OP: 00-03; line: 02-03 HIT; out1: 11  X ; out2: 15  X ; out3: 19  X ; out4: 23  X ; out5: 04 HIT
  - 04; OP: 04-05; line: 04-04 HIT; out1: 06 HIT; out2: 11  X
  - 06; OP: 06-07; line: 07-07 HIT; out1: 08  X ; out2: 15 HIT
  - 08; OP: 08-09; line: 10-10  X ; out1: 10  X ; out2: 19  X
  - 10; OP: 10-10; line: 10-10  X ; out1: 23  X
  - 11; OP: 11-14; line: 05-06  X ; out1: 27  X
  - 15; OP: 15-18; line: 08-09 HIT; out1: 27 HIT
  - 19; OP: 19-22; line: 11-12  X ; out1: 27  X
  - 23; OP: 23-26; line: 14-15  X ; out1: 27  X
  - 27; OP: 27-27; line: 18-18 HIT; out1: EX  X
- paths
  - 0 11 27:  X
  - 0 15 27:  X
  - 0 19 27:  X
  - 0 23 27:  X
  - 0 4 6 8 10 23 27:  X
  - 0 4 6 8 19 27:  X
  - 0 4 6 15 27: HIT
  - 0 4 11 27:  X
