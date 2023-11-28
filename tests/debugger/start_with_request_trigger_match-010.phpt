--TEST--
Starting Debugger: trigger, trigger does not match against multiple (XDEBUG_SESSION)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; !win');
?>
--ENV--
XDEBUG_SESSION=value3
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$xdebugLogFileName = sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'start_with_request_trigger_match-009.txt';
@unlink( $xdebugLogFileName );

dbgpRunFile(
	dirname(__FILE__) . '/empty-echo.inc',
	['step_into', 'step_into', 'property_get -n $e', 'detach'],
	[
		'xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'trigger',
		'xdebug.trigger_value' => 'value1,value2', 'variables_order' => 'PGCS',
		'xdebug.log' => $xdebugLogFileName, 'xdebug.log_level' => 10,
		'xdebug.control_socket' => 'off',
	],
	['timeout' => 1]
);

echo file_get_contents( $xdebugLogFileName );
@unlink( $xdebugLogFileName );
?>
--EXPECTF--
Hi!




[%d] Log opened at %s
[%d] [Config] DEBUG: Checking if trigger 'XDEBUG_TRIGGER' is enabled for mode 'debug'
[%d] [Config] INFO: Trigger value for 'XDEBUG_TRIGGER' not found, falling back to 'XDEBUG_SESSION'
[%d] [Config] DEBUG: The shared secret (xdebug.trigger_value) is multi-value for mode 'debug'
[%d] [Config] WARN: The trigger value 'value3', as set through 'XDEBUG_SESSION', did not match any of the shared secrets (xdebug.trigger_value) for mode 'debug'
[%d] Log closed at %s
