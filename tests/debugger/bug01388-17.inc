<?php
class FooException extends Exception
{
}

class FooBarException extends FooException
{
}

class FooBarBazException extends FooBarException
{
}

class FlubberException extends FooBarBazException
{
}

echo strlen("Hello there!"), "\n";

/* Set -x FooException */
try {throw new FlubberException("this should break", 39); } catch (Exception $e) {}

echo strrev("Hello again!"), "\n";

?>
