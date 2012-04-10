<?php
class Foo {
    static $domains = array('a' => 'b');

    static public function &getDomain($domain) {
       return self::$domains[$domain];
    }
}

Foo::getDomain('a');
