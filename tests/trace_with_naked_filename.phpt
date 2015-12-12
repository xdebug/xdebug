--TEST--
Test trace with "naked filename"
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
--FILE--
<?php
$tf = xdebug_start_trace(sys_get_temp_dir() . '/bug971', XDEBUG_TRACE_COMPUTERIZED);
echo $tf, "\n";
xdebug_stop_trace();
unlink($tf);

$tf = xdebug_start_trace(sys_get_temp_dir() . '/bug971', XDEBUG_TRACE_COMPUTERIZED | XDEBUG_TRACE_NAKED_FILENAME);
echo $tf, "\n";
xdebug_stop_trace();
unlink($tf);
?>
--EXPECTF--
%sbug971.xt
%sbug971
