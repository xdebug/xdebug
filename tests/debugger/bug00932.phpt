--TEST--
Test for bug #932: Show an error if Xdebug can't open the remote debug log
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; !win');
?>
--INI--
xdebug.mode=develop
xdebug.log=/doesnotexist/bug932.log
--FILE--
<?php
?>
--EXPECTF--
Xdebug: [Log Files] File '/doesnotexist/bug932.log' could not be opened.
