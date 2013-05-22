--TEST--
Test for bug #947: Newlines converted when html_errors = 0 (plain)
--INI--
html_errors=0
xdebug.overload_var_dump=1
xdebug.cli_color=0
xdebug.default_enable=1
--FILE--
<?php
$t = "\0" . 'aze
rty\r\nqwerty';
var_dump($t, 4.34);
?>
--EXPECT--
string(18) "\000aze
rty\r\nqwerty"
double(4.34)
