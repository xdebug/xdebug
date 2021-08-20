--TEST--
Test with eval()
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
require_once 'capture-trace.inc';

function bar()
{
	return "bar";
}

function foo()
{
	return bar();
}

foo();

eval("\$foo = foo();\nbar();\nfoo();\n");
echo $foo, "\n";
xdebug_stop_trace();
?>
--EXPECTF--
bar
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d     -> foo() %stest18.php:14
%w%f %w%d       -> bar() %stest18.php:11
%w%f %w%d     -> eval('$foo = foo();\nbar();\nfoo();\n') %stest18.php:16
%w%f %w%d       -> foo() %stest18.php(16) : eval()'d code:1
%w%f %w%d         -> bar() %stest18.php:11
%w%f %w%d       -> bar() %stest18.php(16) : eval()'d code:2
%w%f %w%d       -> foo() %stest18.php(16) : eval()'d code:3
%w%f %w%d         -> bar() %stest18.php:11
%w%f %w%d     -> xdebug_stop_trace() %stest18.php:18
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
