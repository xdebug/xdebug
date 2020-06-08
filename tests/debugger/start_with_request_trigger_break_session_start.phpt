--TEST--
Starting Debugger: trigger, break, XDEBUG_SESSION_START
--SKIPIF--
<?php print "skip Can only be tested manually, as two connections need to be made and the test harnass does not support that"; ?>
--ENV--
XDEBUG_SESSION_START=foobar
--FILE--
<?php
require 'dbgp/dbgpclient.php';

dbgpRunFile(
	dirname(__FILE__) . '/break-echo.inc',
	['stack_get', 'step_into', 'detach', 'stack_get', 'detach'],
	['xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'trigger']
);
?>
--EXPECTF--
