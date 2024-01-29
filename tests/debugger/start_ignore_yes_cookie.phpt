--TEST--
Starting Debugger: overridden COOKIE ignore value is 'yes'
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$xdebugLogFileName = sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'start_ignore_yes_env.txt';
@unlink( $xdebugLogFileName );

dbgpRunFile(
	dirname(__FILE__) . '/break-echo.inc',
	['stack_get', 'step_into', 'detach'],
	[
		'xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'yes',
		'xdebug.log' => $xdebugLogFileName, 'xdebug.log_level' => 10,
	],
	['timeout' => 1, 'env' => [ 'XDEBUG_IGNORE' => 'yes' ], 'auto_prepend' => '<?php $_COOKIE["XDEBUG_IGNORE"] = "yes";']
);

echo file_get_contents( $xdebugLogFileName );
@unlink( $xdebugLogFileName );
?>
--EXPECTF--
Hi!




[%d] Log opened at %s%A
[%d] [Step Debug] DEBUG: Not activating because an 'XDEBUG_IGNORE' ENV variable is present, with value 'yes'.
[%d] [Step Debug] DEBUG: Not activating because an 'XDEBUG_IGNORE' COOKIE variable is present, with value 'yes'.
[%d] Log closed at %s
