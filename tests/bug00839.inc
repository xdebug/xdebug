<?php

class A
{
    const CONSTANTVAR_ONE = 1;
    const CONSTANTVAR_TWO = 2;
    const CONSTANTVAR_THREE = 3;

    static private $_dumpTypeValues = null;

    protected $prop1 = null;
    protected $prop2 = null;

    public function __construct()
    {
    }

    static public function getDumpTypeChoices()
    {
        if (self::$_dumpTypeValues == null) {
            self::$_dumpTypeValues = array();
            $oClass = new ReflectionClass('A');
            $classConstants = $oClass->getConstants();
            $constantPrefix = "CONSTANTVAR_";
            foreach ($classConstants as $key => $val) {
                if (substr($key, 0, strlen($constantPrefix)) === $constantPrefix) {
                    self::$_dumpTypeValues[$val] = strtolower(substr($key, strlen($constantPrefix)));
                }
            }
        }

        return self::$_dumpTypeValues;
    }
}

echo "works not";
$a = new A();
$a->getDumpTypeChoices();


// Set Breakpoint on sleep and inspect $a
sleep(1);
