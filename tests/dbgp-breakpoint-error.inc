<?php
error_reporting(E_ALL);

trigger_error("FOO");

class FooException extends Exception
{
}

throw new FooException("testing foo exception");
?>
