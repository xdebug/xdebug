--TEST--
Test with eval()
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=1
--FILE--
<?php
$tf = xdebug_start_trace('/tmp/'. uniqid('xdt', TRUE));

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
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
bar
Version: 3.%s
File format: %d
TRACE START [%d-%d-%d %d:%d:%d]
2	2	1	%f	%d
2	3	0	%f	%d	foo	1		%stest18a.php	14	0
3	4	0	%f	%d	bar	1		%stest18a.php	11	0
3	4	1	%f	%d
2	3	1	%f	%d
2	5	0	%f	%d	eval	1	'$foo = foo();\nbar();\nfoo();\n'	%stest18a.php	16	0
3	6	0	%f	%d	foo	1		%stest18a.php(16) : eval()'d code	1	0
4	7	0	%f	%d	bar	1		%stest18a.php	11	0
4	7	1	%f	%d
3	6	1	%f	%d
3	8	0	%f	%d	bar	1		%stest18a.php(16) : eval()'d code	2	0
3	8	1	%f	%d
3	9	0	%f	%d	foo	1		%stest18a.php(16) : eval()'d code	3	0
4	10	0	%f	%d	bar	1		%stest18a.php	11	0
4	10	1	%f	%d
3	9	1	%f	%d
2	5	1	%f	%d
2	11	0	%f	%d	file_get_contents	0		%stest18a.php	18	1	'/tmp/xdt%s.xt'
