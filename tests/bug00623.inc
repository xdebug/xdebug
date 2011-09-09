<?php

class testclassDaddy {
    static private $daddyPriv = array(1, 2, 3, 9);
}

class testclass extends testclassDaddy {
    static protected $nameProt;
    static private $namePriv;
    static public function show_name() {
        self::$nameProt = new testclass;
    }
    
    public function show_name_dyn() {
        //do something with $name
        self::$namePriv = array( "Sir John\n", self::$namePriv );
		return self::$namePriv;
    }
}

testclass::show_name();

$t = new testclass;
var_dump($t->show_name_dyn());
var_dump($t->show_name_dyn());

?>
