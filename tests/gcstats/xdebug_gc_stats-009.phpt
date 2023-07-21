--TEST--
GC Stats: Starting with Zend GC off
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.3');
?>
--INI--
xdebug.mode=gcstats
xdebug.start_with_request=yes
zend.enable_gc=0
xdebug.log=
xdebug.log_level=10
--FILE--
<?php
?>
--EXPECTF--
Xdebug: [GC Stats] PHP's Garbage Collection is disabled
