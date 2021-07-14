--TEST--
Starting Debugger: trigger, trigger does not match [2]
--ENV--
XDEBUG_SESSION=mySession
--FILE--
<?php
require 'dbgp/dbgpclient.php';

dbgpRunFile(
	dirname(__FILE__) . '/empty-echo.inc',
	['step_into', 'step_into', 'property_get -n $e', 'detach'],
	['xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'trigger', 'xdebug.trigger_value' => 'yourSession', 'variables_order' => 'PGCS'],
	['timeout' => 1]
);
?>
--EXPECTF--
Hi!
