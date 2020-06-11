--TEST--
Starting Debugger: default, no environment
--FILE--
<?php
require 'dbgp/dbgpclient.php';
dbgpRunFile(
	dirname(__FILE__) . '/empty-echo.inc',
	['step_into', 'step_into', 'property_get -n $e', 'detach'],
	['xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'default'],
	['timeout' => 1]
);
?>
--EXPECT--
Hi!
