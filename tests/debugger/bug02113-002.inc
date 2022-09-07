<?php
function g(){
    return 2;
}
function myMethod($b)
{
	try {
	    f(); //breakpoint there
	} catch (Throwable $e) {
	}
	return "We have returned";
}

$foo = 1;
$a = myMethod(42);

$floob = g();
