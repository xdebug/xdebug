--TEST--
Test for bug #422: Segfaults when using code coverage with a parse error in the script
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.trace_options=0
xdebug.trace_output_dir=/tmp
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
xdebug.extended_info=1
--FILE--
<?php
function hdl(){}
set_error_handler('hdl');
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);
require_once 'bug00422.inc';
echo "END";
?>
--EXPECT--
END
