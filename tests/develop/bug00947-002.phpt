--TEST--
Test for bug #947: Newlines converted when html_errors = 0 (plain)
--INI--
html_errors=0
xdebug.mode=develop
xdebug.cli_color=0
--FILE--
<?php
$t = "\0" . 'aze
rty\r\nqwerty';
var_dump($t, 4.34);
?>
--EXPECTF--
%sbug00947-002.php:4:
string(%r(18|19)%r) "\000aze
rty\r\nqwerty"
%sbug00947-002.php:4:
double(4.34)
