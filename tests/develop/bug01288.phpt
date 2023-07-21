--TEST--
Test for bug #1288: Segfault when uncaught exception message does not contain " in "
--INI--
xdebug.mode=develop
html_errors=0
--FILE--
<?php

class React_Exception extends Exception
{
    public function __toString()
    {
        return "Custom message";
        return "Custom message in random"; // this does not segfault because of ' in '.
    }
}

set_error_handler(function(){ throw new React_Exception('waa');});


$_SERVER['SERVER_PROTOCOL'];
?>
--EXPECTF--
Fatal error: Uncaught Custom message
  thrown in %sbug01288.php on line 12

React_Exception: waa in %sbug01288.php on line 12

Call Stack:
%w%f %w%d   1. {main}() %sbug01288.php:0
%w%f %w%d   2. {closure:%sbug01288.php:12-12}(%d, 'Undefined %SSERVER_PROTOCOL%S', '%sbug01288.php', %s) %sbug01288.php:15
