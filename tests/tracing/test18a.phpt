--TEST--
Test with eval()
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.trace_format=1
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
Version: %d.%s
File format: %d
TRACE START [%d-%d-%d %d:%d:%d.%d]
2	%d	1	%f	%d
2	%d	0	%f	%d	foo	1		%stest18a.php	14	0
3	%d	0	%f	%d	bar	1		%stest18a.php	11	0
3	%d	1	%f	%d
2	%d	1	%f	%d
2	%d	0	%f	%d	eval	1	'$foo = foo();\nbar();\nfoo();\n'	%stest18a.php	16	0
3	%d	0	%f	%d	foo	1		%stest18a.php(16) : eval()'d code	1	0
4	%d	0	%f	%d	bar	1		%stest18a.php	11	0
4	%d	1	%f	%d
3	%d	1	%f	%d
3	%d	0	%f	%d	bar	1		%stest18a.php(16) : eval()'d code	2	0
3	%d	1	%f	%d
3	%d	0	%f	%d	foo	1		%stest18a.php(16) : eval()'d code	3	0
4	%d	0	%f	%d	bar	1		%stest18a.php	11	0
4	%d	1	%f	%d
3	%d	1	%f	%d
2	%d	1	%f	%d
2	%d	0	%f	%d	xdebug_stop_trace	0		%stest18a.php	18	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
