--TEST--
Test for somewhat more complex backtrace
--INI--
xdebug.enable=1
--FILE--
<?php

function fucking() {
    deep();
}

function deep() {
    nested();
}

function nested() {
    error();
}

function error() {
    in();
}

function in() {
    func();
}

function func() {
    blah();
}

function blah() {
    funky_shit();
}

fucking();

?>
--EXPECTF--
Fatal error: Call to undefined function:  funky_shit() in /%s/phpt.%x on line 28

Call Stack:
    %f      %i   1. {main}() /%s/phpt.%x:0
    %f      %i   2. fucking() /%s/phpt.%x:31
    %f      %i   3. deep() /%s/phpt.%x:4
    %f      %i   4. nested() /%s/phpt.%x:8
    %f      %i   5. error() /%s/phpt.%x:12
    %f      %i   6. in() /%s/phpt.%x:16
    %f      %i   7. func() /%s/phpt.%x:20
    %f      %i   8. blah() /%s/phpt.%x:24
