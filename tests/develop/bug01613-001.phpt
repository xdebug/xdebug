--TEST--
Test for bug #1613: Wrong name displayed for Recoverable fatal error [text] (< PHP 7.4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 7.4');
?>
--INI--
xdebug.mode=develop
xdebug.collect_assignments=0
xdebug.show_local_vars=0
xdebug.dump_globals=0
--FILE--
<?php
$v = new DateTime();
$v = (string) $v;
?>
--EXPECTF--
Recoverable fatal error: Object of class DateTime could not be converted to string in %sbug01613-001.php on line %d

Call Stack:
%w%f  %w%d   1. {main}() %sbug01613-001.php:%d
