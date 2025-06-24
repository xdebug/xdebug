--TEST--
Test foreach with Code Coverage with path and branch checking
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--INI--
xdebug.mode=coverage
xdebug.trace_options=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.dump_globals=0
xdebug.trace_format=0
--FILE--
<?php
include 'dump-branch-coverage.inc';

$a1 = [];

$a2 = [ "welsh", "gaelic", "english" ];

$i1 = new DatePeriod(
	new DateTimeImmutable("2024-01-14"),
	DateInterval::createFromDateString("+1 day"),
	new DateTimeImmutable("2024-01-01")
);

include 'foreach.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);
showLanguages( $a1 );
showLanguages( $a2 );
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
xdebug_stop_code_coverage();

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);
showLanguages( $i1 );
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
xdebug_stop_code_coverage();
?>
--EXPECT--
welsh
gaelic
english
showLanguages
- branches
  - 00; OP: 00-02; line: 02-04 HIT; out1: 03 HIT; out2: 09  X
  - 03; OP: 03-03; line: 04-04 HIT; out1: 04 HIT; out2: 09 HIT
  - 04; OP: 04-08; line: 05-04 HIT; out1: 03 HIT
  - 09; OP: 09-11; line: 04-07 HIT; out1: EX  X
- paths
  - 0 3 4 3 9: HIT
  - 0 3 9: HIT

showLanguages
- branches
  - 00; OP: 00-02; line: 02-04 HIT; out1: 03  X ; out2: 09 HIT
  - 03; OP: 03-03; line: 04-04  X ; out1: 04  X ; out2: 09  X
  - 04; OP: 04-08; line: 05-04  X ; out1: 03  X
  - 09; OP: 09-11; line: 04-07 HIT; out1: EX  X
- paths
  - 0 3 4 3 9:  X
  - 0 3 9:  X
  - 0 9: HIT
