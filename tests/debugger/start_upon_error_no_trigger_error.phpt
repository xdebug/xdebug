--TEST--
Starting Debugger: trigger, error, start_up_error=no
--FILE--
<?php
require 'dbgp/dbgpclient.php';

dbgpRunFile(
	dirname(__FILE__) . '/break-with-error.inc',
	['stack_get', 'step_into', 'detach'],
	['xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'trigger', 'xdebug.start_upon_error' => 'no'],
	['timeout' => 1]
);
?>
--EXPECTF--
Not triggered!
