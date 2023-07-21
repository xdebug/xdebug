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
2	1	1	%f	%d
2	7	0	%f	%d	foo	1		%stest18a.php	14	0
3	8	0	%f	%d	bar	1		%stest18a.php	11	0
3	8	1	%f	%d
2	7	1	%f	%d
2	9	0	%f	%d	eval	1	'$foo = foo();\nbar();\nfoo();\n'	%stest18a.php	16	0
3	10	0	%f	%d	foo	1		%stest18a.php(16) : eval()'d code	1	0
4	11	0	%f	%d	bar	1		%stest18a.php	11	0
4	11	1	%f	%d
3	10	1	%f	%d
3	12	0	%f	%d	bar	1		%stest18a.php(16) : eval()'d code	2	0
3	12	1	%f	%d
3	13	0	%f	%d	foo	1		%stest18a.php(16) : eval()'d code	3	0
4	14	0	%f	%d	bar	1		%stest18a.php	11	0
4	14	1	%f	%d
3	13	1	%f	%d
2	9	1	%f	%d
2	15	0	%f	%d	xdebug_stop_trace	0		%stest18a.php	18	0
			%f	%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]
