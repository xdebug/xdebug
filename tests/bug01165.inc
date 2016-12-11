<?php

function bar()
{
    return 42;
}

function foo($arg)
{
    echo $arg;
}

foo(bar());
echo 6 * 9;

?>
