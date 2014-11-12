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

class MoonException extends Exception
{
}

echo strlen("Hello there!"), "\n";

/* Set -x FooBarException */
try {throw new MoonException("this should not break"); } catch (Exception $e) {}
try {throw new FooException("this should not break"); } catch (Exception $e) {}
try {throw new FooBarException("this should break", 42); } catch (Exception $e) {}

/* Set -x FooException */
try {throw new MoonException("this should not break"); } catch (Exception $e) {}
try {throw new FooException("this should now break"); } catch (Exception $e) {}
try {throw new FooBarException("this should still break", 43); } catch (Exception $e) {}

/* Set -x * */
try {throw new MoonException("this should now break"); } catch (Exception $e) {}
?>
