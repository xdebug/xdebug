--TEST--
DBGP: skip vendor directory
--SKIPIF--
<?php
require __DIR__ . '/../../../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require __DIR__ . '/../../dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/skip-vendor-directory.inc';

$xdebugLogFileName = sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'start_ignore_yes_env.txt';
@unlink( $xdebugLogFileName );

$commands = array(
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
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
-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://skip-vendor-directory.inc" lineno="6"></xdebug:message></response>

-> step_into -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://counter.inc" lineno="15"></xdebug:message></response>

-> step_into -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://counter.inc" lineno="6"></xdebug:message></response>
%A
[%d] [Path Mapping] INFO: Scanning for map files with pattern '%sdebugger%e.xdebug%e*.map'
[%d] [Path Mapping] DEBUG: No map files found with pattern '%sdebugger%e.xdebug%e*.map'
[%d] [Path Mapping] INFO: Scanning for map files with pattern '%sdebugger%emaps%e.xdebug%e*.map'
[%d] [Path Mapping] DEBUG: No map files found with pattern '%sdebugger%emaps%e.xdebug%e*.map'
[%d] [Path Mapping] INFO: Scanning for map files with pattern '%sdebugger%emaps%eskip-vendor-directory%e.xdebug%e*.map'
[%d] [Path Mapping] INFO: Reading mapping file '%sdebugger%emaps%eskip-vendor-directory%e.xdebug%evendor.map'
[%d] [Path Mapping] DEBUG: Found 1 path mapping rules
%A
[%d] [Step Debug] <- step_into -i 3
[%d] [Path Mapping] INFO: Mapping location %svendor%eautoload.php:6
[%d] [Path Mapping] INFO: Location %svendor%eautoload.php:6 needs to be skipped
[%d] [Path Mapping] INFO: Mapping location %svendor%eautoload.php:7
[%d] [Path Mapping] INFO: Location %svendor%eautoload.php:7 needs to be skipped
[%d] [Path Mapping] INFO: Mapping location %sxdebug%ecounter.inc:15
[%d] [Path Mapping] INFO: Couldn't map location %sxdebug%ecounter.inc:15
[%d] [Step Debug] -> <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://%sxdebug%ecounter.inc" lineno="15"></xdebug:message></response>
%A
[%d] [Step Debug] <- step_into -i 4
[%d] [Path Mapping] INFO: Mapping location %svendor%eautoload.php:8
[%d] [Path Mapping] INFO: Location %svendor%eautoload.php:8 needs to be skipped
[%d] [Path Mapping] INFO: Mapping location %sxdebug%ecounter.inc:6
[%d] [Path Mapping] INFO: Couldn't map location %sxdebug%ecounter.inc:6
[%d] [Step Debug] -> <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://%sxdebug%ecounter.inc" lineno="6"></xdebug:message></response>
%A
