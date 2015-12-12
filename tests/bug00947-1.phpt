--TEST--
Test for bug #947: Newlines converted when html_errors = 0 (ansi)
--SKIPIF--
<?php if (substr(PHP_OS, 0, 3) == "WIN") die("skip Not for Windows"); ?>
--INI--
html_errors=0
xdebug.overload_var_dump=1
xdebug.cli_color=2
xdebug.default_enable=1
--FILE--
<?php
$t = "\0" . 'aze
rty\r\nqwerty';
var_dump($t, 4.34);
?>
--EXPECTF--
[1mstring[22m([32m18[0m) "[31m\000aze\nrty\\r\\nqwerty[0m"
[1mdouble[22m([33m4.34[0m)
