--TEST--
Test with eval()
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
xdebug.collect_return=0
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
    %f      %d     -> foo() /%s/test18.php:14
    %f      %d       -> bar() /%s/test18.php:11
    %f      %d     -> eval('$foo = foo();
bar();
foo();
') /%s/test18.php:16
    %f      %d       -> foo() /%s/test18.php(16) : eval()'d code:1
    %f      %d         -> bar() /%s/test18.php:11
    %f      %d       -> bar() /%s/test18.php(16) : eval()'d code:2
    %f      %d       -> foo() /%s/test18.php(16) : eval()'d code:3
    %f      %d         -> bar() /%s/test18.php:11
    %f      %d     -> file_get_contents('/tmp/%s') /%s/test18.php:18
