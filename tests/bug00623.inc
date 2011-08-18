<?php

class testclass {
    static private $namePriv;
    static protected $nameProt;
    static public $namePub;
    
    static public function show_name() {
        //do something with $name
        self::$namePriv = array( "Sir John\n", self::$namePriv );
        self::$nameProt = self::$namePriv;
        self::$namePub  = self::$namePriv;
        return self::$namePriv;
    }
    
    public function show_name_dyn() {
        //do something with $name
        self::$namePriv = array( "Sir John\n", self::$namePriv );
        self::$nameProt = self::$namePriv;
        self::$namePub  = self::$namePriv;
        return self::$namePriv;
    }
}

echo testclass::show_name();
echo testclass::show_name();
echo testclass::show_name();

$t = new testclass;
$t->show_name_dyn();

?>
