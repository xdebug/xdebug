--TEST--
Starting Debugger: never, XDEBUG_CONFIG=idekey=derickr
--ENV--
XDEBUG_CONFIG=idekey=derickr
--FILE--
<?php
require 'dbgp/dbgpclient.php';

dbgpRunFile(
	dirname(__FILE__) . '/empty-echo.inc',
	['step_into', 'step_into', 'property_get -n $e', 'detach'],
	['xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'no'],
	['timeout' => 1]
);
?>
--EXPECTF--
Hi!
