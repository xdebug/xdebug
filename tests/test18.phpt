--TEST--
Test with eval()
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
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
TRACE START [%d-%d-%d %d:%d:%d]
%w%f %w%d     -> foo() /%s/test18.php:14
%w%f %w%d       -> bar() /%s/test18.php:11
%w%f %w%d     -> eval('$foo = foo();\nbar();\nfoo();\n') /%s/test18.php:16
%w%f %w%d       -> foo() /%s/test18.php(16) : eval()'d code:1
%w%f %w%d         -> bar() /%s/test18.php:11
%w%f %w%d       -> bar() /%s/test18.php(16) : eval()'d code:2
%w%f %w%d       -> foo() /%s/test18.php(16) : eval()'d code:3
%w%f %w%d         -> bar() /%s/test18.php:11
%w%f %w%d     -> file_get_contents('/tmp/%s') /%s/test18.php:18
