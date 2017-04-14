<?php
class Bar {
    static $var = 52;
    public static function f2() {
        foo();
    }
}

function foo() {
    echo "hi"; // put breakpoint here
}

Bar::f2();
