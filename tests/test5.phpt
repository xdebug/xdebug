--TEST--
Test for somewhat more complex backtrace
--INI--
xdebug.enable=1
xdebug.dump_globals=0
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
Fatal error: Call to undefined function%sfunky_shit() in /%s/test5.php on line 28

Call Stack:
    %f      %i   1. {main}() /%s/test5.php:0
    %f      %i   2. fucking() /%s/test5.php:31
    %f      %i   3. deep() /%s/test5.php:4
    %f      %i   4. nested() /%s/test5.php:8
    %f      %i   5. error() /%s/test5.php:12
    %f      %i   6. in() /%s/test5.php:16
    %f      %i   7. func() /%s/test5.php:20
    %f      %i   8. blah() /%s/test5.php:24
