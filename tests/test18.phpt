--TEST--
Test with eval()
--INI--
xdebug.enable=1
xdebug.auto_trace=0
xdebug.collect_params=1
--FILE--
<?php
xdebug_start_trace();

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
echo $foo;
xdebug_dump_function_trace();
?>
--EXPECTF--
bar
Function trace:
    %f      %d     -> foo() /%s/test18.php:14
    %f      %d       -> bar() /%s/test18.php:11
    %f      %d     -> eval($foo = foo();
bar();
foo();
) /%s/test18.php:16
    %f      %d       -> foo() /%s/test18.php(16) : eval()'d code:1
    %f      %d         -> bar() /%s/test18.php:11
    %f      %d       -> bar() /%s/test18.php(16) : eval()'d code:2
    %f      %d       -> foo() /%s/test18.php(16) : eval()'d code:3
    %f      %d         -> bar() /%s/test18.php:11
