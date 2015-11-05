--TEST--
Test with Code Coverage with path and branch checking (< PHP 7.0)
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.trace_options=0
xdebug.trace_output_dir=/tmp
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
xdebug.extended_info=1
xdebug.overload_var_dump=0
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
--FILE--
<?php
include 'dump-branch-coverage.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

include 'coverage6.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
?>
--EXPECT--
A NOT B
B NOT A
1
2
1
loop_test
- branches
  - 00; OP: 00-02; line: 12-15 HIT; out1: 03 HIT
  - 03; OP: 03-07; line: 15-16 HIT; out1: 08 HIT; out2: 03 HIT
  - 08; OP: 08-09; line: 17-17 HIT; out1: EX  X 
- paths
  - 0 3 8: HIT
  - 0 3 3 8: HIT

ok
- branches
  - 00; OP: 00-04; line: 02-04 HIT; out1: 05 HIT; out2: 07 HIT
  - 05; OP: 05-06; line: 04-04 HIT; out1: 07 HIT
  - 07; OP: 07-07; line: 04-04 HIT; out1: 08 HIT; out2: 11 HIT
  - 08; OP: 08-10; line: 05-06 HIT; out1: 11 HIT
  - 11; OP: 11-13; line: 07-07 HIT; out1: 14 HIT; out2: 15 HIT
  - 14; OP: 14-14; line: 07-07 HIT; out1: 15 HIT
  - 15; OP: 15-15; line: 07-07 HIT; out1: 16 HIT; out2: 19 HIT
  - 16; OP: 16-18; line: 08-09 HIT; out1: 19 HIT
  - 19; OP: 19-20; line: 10-10 HIT; out1: EX  X 
- paths
  - 0 5 7 8 11 14 15 16 19:  X 
  - 0 5 7 8 11 14 15 19:  X 
  - 0 5 7 8 11 15 16 19:  X 
  - 0 5 7 8 11 15 19: HIT
  - 0 5 7 11 14 15 16 19:  X 
  - 0 5 7 11 14 15 19:  X 
  - 0 5 7 11 15 16 19:  X 
  - 0 5 7 11 15 19: HIT
  - 0 7 8 11 14 15 16 19:  X 
  - 0 7 8 11 14 15 19:  X 
  - 0 7 8 11 15 16 19:  X 
  - 0 7 8 11 15 19:  X 
  - 0 7 11 14 15 16 19: HIT
  - 0 7 11 14 15 19: HIT
  - 0 7 11 15 16 19:  X 
  - 0 7 11 15 19:  X 

{main}
- branches
  - 00; OP: 00-39; line: 02-26 HIT; out1: EX  X 
- paths
  - 0: HIT
