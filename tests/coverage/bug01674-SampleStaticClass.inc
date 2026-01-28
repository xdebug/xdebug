<?php
declare(strict_types=1);

namespace Xdebug;

class SampleStaticClass
{
    private static $field0 = '0';
    private static $field1 = '1';
    private static $field2 = '2';
    private static $field3 = '3';

    public static function getField0(): string
    {
        return self::$field0;
    }

    public static function getField1(): string
    {
        return self::$field1;
    }

    public static function getField2(): string
    {
        return self::$field2;
    }

    public static function getField3(): string
    {
        return self::$field3;
    }

    public function getField0NonStatic(): string
    {
        return self::$field0;
    }

    public function getField1NonStatic(): string
    {
        return self::$field1;
    }

    public function getField2NonStatic(): string
    {
        return self::$field2;
    }

    public function getField3NonStatic(): string
    {
        return self::$field3;
    }
}
