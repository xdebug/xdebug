--TEST--
Starting Debugger: overridden GET ignore value is 'yes'
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$xdebugLogFileName = getTmpFile('start_ignore_yes_env.txt');
@unlink( $xdebugLogFileName );

dbgpRunFile(
	dirname(__FILE__) . '/break-echo.inc',
	['stack_get', 'step_into', 'detach'],
	[
		'xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'yes',
		'xdebug.log' => $xdebugLogFileName, 'xdebug.log_level' => 10,
	],
	['timeout' => 1, 'env' => [ 'XDEBUG_IGNORE' => 'yes' ], 'auto_prepend' => '<?php $_GET["XDEBUG_IGNORE"] = "yes";']
);

echo file_get_contents( $xdebugLogFileName );
@unlink( $xdebugLogFileName );
?>
--EXPECTF--
Hi!




[%d] Log opened at %s%A
[%d] [Step Debug] DEBUG: Not activating because an 'XDEBUG_IGNORE' ENV variable is present, with value 'yes'.
[%d] [Step Debug] DEBUG: Not activating because an 'XDEBUG_IGNORE' GET variable is present, with value 'yes'.
[%d] Log closed at %s
