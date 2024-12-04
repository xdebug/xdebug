--TEST--
DBGP: line breakpoint with path map
--SKIPIF--
<?php
require __DIR__ . '/../../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require __DIR__ . '/../dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/01/dbgp-breakpoint-line.inc';

$xdebugLogFileName = sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'start_ignore_yes_env.txt';
@unlink( $xdebugLogFileName );

$commands = array(
	'feature_set -n breakpoint_details -v 1',
	'step_into',
	'breakpoint_set -t line -n 3',
	'run',
	'detach',
);

dbgpRunFile(
	$filename, $commands,
	[
		'xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'yes',
		'xdebug.log' => $xdebugLogFileName, 'xdebug.log_level' => 10,
		'xdebug.path_mapping' => 'yes',
	]
);

echo file_get_contents( $xdebugLogFileName );
@unlink( $xdebugLogFileName );
?>
--EXPECTF--
%A
[%d] [Path Mapping] INFO: Scanning for map files with pattern '%sdebugger/.xdebug/*.map'
[%d] [Path Mapping] DEBUG: No map files found with pattern '%sdebugger/.xdebug/*.map'
[%d] [Path Mapping] INFO: Scanning for map files with pattern '%sdebugger/maps/.xdebug/*.map'
[%d] [Path Mapping] DEBUG: No map files found with pattern '%sdebugger/maps/.xdebug/*.map'
[%d] [Path Mapping] INFO: Scanning for map files with pattern '%sdebugger/maps/01/.xdebug/*.map'
[%d] [Path Mapping] INFO: Reading mapping file '%sdebugger/maps/01/.xdebug/simple.map'
[%d] [Path Mapping] DEBUG: Found 1 path mapping rules
%A
