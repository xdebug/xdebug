--TEST--
Test for bug #470: catch blocks marked as dead code unless executed (< PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
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
--FILE--
<?php
    xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);

    include 'bug00470.inc';
    $cc = xdebug_get_code_coverage();
	ksort($cc);
    var_dump(array_slice($cc, 1, 1));

    new Ticket842;
    $cc = xdebug_get_code_coverage();
	ksort($cc);
    var_dump(array_slice($cc, 1, 1));

    xdebug_stop_code_coverage(false);
?>
--EXPECTF--
array(1) {
  ["%sbug00470.inc"]=>
  array(5) {
    [3]=>
    int(1)
    [8]=>
    int(-1)
    [9]=>
    int(-1)
    [11]=>
    int(-1)
    [14]=>
    int(1)
  }
}
array(1) {
  ["%sbug00470.inc"]=>
  array(5) {
    [3]=>
    int(1)
    [8]=>
    int(1)
    [9]=>
    int(-1)
    [11]=>
    int(1)
    [14]=>
    int(1)
  }
}
