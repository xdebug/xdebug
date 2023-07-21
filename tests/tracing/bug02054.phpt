--TEST--
Test for bug #2054: Incorrectly truncated string
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=1
xdebug.trace_format=0
xdebug.var_display_max_data=9
--FILE--
<?php
require_once 'capture-trace.inc';

$string = str_repeat("'", 64);

xdebug_stop_trace();
?>
--EXPECTF--
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w               => $tf = '%s'... %s
%w%f %w%d     -> str_repeat($%s = '\'', $%s = 64) %sbug02054.php:4
%w               => $string = '\'\'\'\'\'\'\'\'\''... %sbug02054.php:4
%w%f %w%d     -> xdebug_stop_trace() %sbug02054.php:6
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
