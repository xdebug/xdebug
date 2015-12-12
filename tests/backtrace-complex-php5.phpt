--TEST--
Test for somewhat more complex backtrace (< PHP 7.0)
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
--INI--
xdebug.default_enable=1
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.profiler_enable=0
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
Fatal error: Call to undefined function%sfunky_shit() in %sbacktrace-complex-php5.php on line 28

Call Stack:
%w%f %w%d   1. {main}() %sbacktrace-complex-php5.php:0
%w%f %w%d   2. fucking() %sbacktrace-complex-php5.php:31
%w%f %w%d   3. deep() %sbacktrace-complex-php5.php:4
%w%f %w%d   4. nested() %sbacktrace-complex-php5.php:8
%w%f %w%d   5. error() %sbacktrace-complex-php5.php:12
%w%f %w%d   6. in() %sbacktrace-complex-php5.php:16
%w%f %w%d   7. func() %sbacktrace-complex-php5.php:20
%w%f %w%d   8. blah() %sbacktrace-complex-php5.php:24
