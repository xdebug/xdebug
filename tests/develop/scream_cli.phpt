--TEST--
Test for scream support
--INI--
xdebug.mode=develop
xdebug.scream=0
error_reporting(E_ALL);
xdebug.cli_color=2
xdebug.force_error_reporting=0
--FILE--
<?php
echo @hex2bin('4'), "\n";
ini_set('xdebug.scream', 1);
echo @hex2bin('4'), "\n";
ini_set('xdebug.scream', 0);
echo @hex2bin('4'), "\n";
?>
--EXPECTF--
[1m[31mSCREAM[0m:  Error suppression ignored for
[1m[31mWarning[0m: %s[22m in [31m%sscream_cli.php[0m on line [32m4[0m[22m

[1mCall Stack:[22m
%w%f %w%d   1. {main}() %sscream_cli.php:0
%w%f %w%d   2. hex2bin($%s = '4') %sscream_cli.php:4
