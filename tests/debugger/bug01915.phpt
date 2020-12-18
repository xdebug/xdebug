--TEST--
Test for bug #1915: Debugger should only start with XDEBUG_SESSION and not XDEBUG_PROFILE
--ENV--
XDEBUG_PROFILE=1
--FILE--
<?php
require 'dbgp/dbgpclient.php';

dbgpRunFile(
	dirname(__FILE__) . '/empty-echo.inc',
	['step_into', 'step_into', 'property_get -n $e', 'detach'],
	['xdebug.mode' => 'debug,profile', 'xdebug.start_with_request' => 'trigger'],
	['timeout' => 1]
);
?>
--EXPECT--
Hi!
