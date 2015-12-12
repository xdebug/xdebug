--TEST--
Test for bug #932: Show an error if Xdebug can't open the remote debug log.
--SKIPIF--
<?php if (substr(PHP_OS, 0, 3) == "WIN") die("skip Not for Windows"); ?>
--INI--
xdebug.default_enable=1
xdebug.remote_enable=1
xdebug.remote_autostart=1
xdebug.remote_mode=jit
xdebug.remote_log=/tmp/bug932.log
xdebug.force_error_reporting=0
--FILE--
<?php
touch("/tmp/bug932.log");
chmod("/tmp/bug932.log", 0);

@trigger_error('foo');

unlink("/tmp/bug932.log");
?>
--EXPECT--
XDebug could not open the remote debug file '/tmp/bug932.log'.
