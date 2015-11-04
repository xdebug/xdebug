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

include 'coverage5.inc';

xdebug_stop_code_coverage(false);
$c = xdebug_get_code_coverage();
dump_branch_coverage($c);
?>
--EXPECTF--
A NOT B
{main}
- branches
  - 00; OP: 00-05; line: 02-05 HIT; out1: 06 HIT; out2: 08  X 
  - 06; OP: 06-07; line: 05-05 HIT; out1: 08 HIT
  - 08; OP: 08-08; line: 05-05 HIT; out1: 09 HIT; out2: 12  X 
  - 09; OP: 09-11; line: 06-07 HIT; out1: 19 HIT
  - 12; OP: 12-13; line: 07-07  X ; out1: 14  X ; out2: 15  X 
  - 14; OP: 14-14; line: 07-07  X ; out1: 15  X 
  - 15; OP: 15-15; line: 07-07  X ; out1: 16  X ; out2: 19  X 
  - 16; OP: 16-18; line: 08-09  X ; out1: 19  X 
  - 19; OP: 19-20; line: 11-11 HIT; out1: EX  X 
- paths
  - 0 6 8 9 19: HIT
  - 0 6 8 12 14 15 16 19:  X 
  - 0 6 8 12 14 15 19:  X 
  - 0 6 8 12 15 16 19:  X 
  - 0 6 8 12 15 19:  X 
  - 0 8 9 19:  X 
  - 0 8 12 14 15 16 19:  X 
  - 0 8 12 14 15 19:  X 
  - 0 8 12 15 16 19:  X 
  - 0 8 12 15 19:  X
