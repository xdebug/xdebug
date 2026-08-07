--TEST--
Test trace with "naked filename"
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=0
xdebug.use_compression=0
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

$tf = xdebug_start_trace(getTmpFile('bug971' . uniqid('', true)), XDEBUG_TRACE_COMPUTERIZED);
echo $tf, "\n";
xdebug_stop_trace();
unlink($tf);

$tf = xdebug_start_trace(getTmpFile('bug971' . uniqid('', true)), XDEBUG_TRACE_COMPUTERIZED | XDEBUG_TRACE_NAKED_FILENAME);
echo $tf, "\n";
xdebug_stop_trace();
unlink($tf);
?>
--EXPECTF--
%sbug971%s.xt
%sbug971%s
