--TEST--
Test for bug #823: Single quotes are escaped in var_dumped string output
--INI--
xdebug.cli_color=0
--FILE--
<?php
var_dump("'hello world'");
?>
--EXPECT--
string(13) "'hello world'"
