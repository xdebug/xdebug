<?php

function bar()
{
	return "bar";
}

function foo()
{
	return bar();
}

xdebug_start_trace();
foo();

eval("\$foo = foo();\nbar();\nfoo();\n");
echo $foo;
xdebug_dump_function_trace();
?>

