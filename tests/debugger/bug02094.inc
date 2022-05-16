<?php

class Test {
    public static $bar;

    public static function foo($closure) {
        static::$bar = $closure;
    }

    public static function baz() {
        (static::$bar)();
    }
}

Test::foo(function(){
    echo "called";
});

Test::baz();
?>
