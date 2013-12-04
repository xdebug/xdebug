<?php
class SimpleClass
{
    const HELLO = "HELLO";
    public static $hello;

    public static function init(){
        return self::displayVar();
    }
    
    private static function displayVar() {
        
        self::$hello = self::HELLO;
        return self::$hello;
    }
}
?>
