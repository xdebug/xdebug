--TEST--
Test for bug #1907: Empty exception message when setting the $message property to a stringable object
--FILE--
<?php
  
class LazyException extends Exception
{
    public function __construct()
    {
        parent::__construct();

        $this->message = new class() {
            public function __toString()
            {
                return 'World';
            }
        };
    }
}

$e = new LazyException();

try {
    throw $e;
} catch (\Exception $e) {
    echo 'Hello ';
    echo $e->getMessage();
    echo "\n";
}
?>
--EXPECT--
Hello World
