<?php
function g(){
    return 2;
}
function myMethod($b)
{
    f(); //breakpoint there
}

$foo = 1;
$a = myMethod(42);
