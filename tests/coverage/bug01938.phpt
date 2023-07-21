--TEST--
Test for bug #1938: Branches in traits aren’t marked as executed (>= PHP 7.4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.4');
?>
--INI--
xdebug.mode=coverage
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK );

require dirname( __FILE__ ) . '/bug01938-FooTrait.inc';
require dirname( __FILE__ ) . '/bug01938-Bar.inc';

$c = new \App\Bar;
$c->useTrait();

$cc = xdebug_get_code_coverage();
dump_branch_coverage($cc);
xdebug_stop_code_coverage();
?>
--EXPECTF--
App\Bar->useTrait
- branches
  - 00; OP: 00-%d; line: %d-%d HIT%S
- paths
  - 0: HIT

{main}
- branches
  - 00; OP: 00-%d; line: %d-16 HIT; out1: EX  X
- paths
  - 0: HIT

returnsTrue{trait-method:%sbug01938-FooTrait.inc:9-12}
- branches
  - 00; OP: 00-%d; line: %d-%d HIT%S
- paths
  - 0: HIT

{main}
- branches
  - 00; OP: 00-%d; line: %d-14 HIT; out1: EX  X
- paths
  - 0: HIT
