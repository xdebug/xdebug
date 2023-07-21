<?php
class TestClass
{
    public static function someMethod($case)
    {
        switch ($case) {
            case 'foo':
                return 1;
                break;
            case 'bar':
                return 2;
                break;
            default:
                return 3;
                break;
        }
    }
}

TestClass::someMethod('foo');
TestClass::someMethod('bar');
TestClass::someMethod('1');

?>
