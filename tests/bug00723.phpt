--TEST--
Test for bug #723: xdebug is stricter than PHP regarding Exception property types
--INI--
html_errors=0
display_errors=1
error_reporting=-1
--FILE--
<?php
class MyException extends Exception {
    function __construct($message) {
        parent::__construct($message);
        $this->line = "42"; // this triggers it. if it's before the constructor, PHP freaks out regardless of xdebug
    }
}

try {
    throw new MyException('test');
} catch (Exception $ex) {
}

echo "survived";
?>
--EXPECTF--
survived
