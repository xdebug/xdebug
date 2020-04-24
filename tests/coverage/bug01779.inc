<?php
namespace MyDebug;

class DebugClass
{
    public static function createNestedThreeTimes(string $string)
    {
        return new self(
            strtolower(
                sprintf(
                    "%s",
                    $string
                )
            )
        );
    }

}

DebugClass::createNestedThreeTimes("testing");
?>
