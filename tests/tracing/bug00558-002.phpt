--TEST--
Test for bug #558: PHP segfaults when running a nested eval while tracing.
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_assignments=0
xdebug.collect_return=0
xdebug.trace_format=0
--FILE--
<?php
require_once 'capture-trace.inc';

$any = 'printf("foo\n");';
eval('eval($any);');

xdebug_stop_trace();
?>
DONE
--EXPECTF--
foo
DONE
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f  %w%d     -> eval('eval($any);') %sbug00558-002.php:5
%w%f  %w%d       -> eval('printf("foo\\n");') %sbug00558-002.php(5) : eval()'d code:1
%w%f  %w%d         -> printf($format = 'foo\n') %sbug00558-002.php(5) : eval()'d code(1) : eval()'d code:1
%w%f  %w%d     -> xdebug_stop_trace() %sbug00558-002.php:7
%w%f  %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
