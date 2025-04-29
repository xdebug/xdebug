--TEST--
Test for bug #2261: Control socket in init package (Control Socket without TSC)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; ext-flag control-socket; !ext-flag tsc; linux');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01949.inc';

$commands = array(
	'detach',
);

$xdebugLogFileName = getTmpFile('remote-log-2261-002.txt');
@unlink( $xdebugLogFileName );

dbgpRunFile( $filename, $commands, [ 'xdebug.log' => $xdebugLogFileName, 'xdebug.log_level' => 7, 'xdebug.control_socket' => 'time' ] );

echo file_get_contents( $xdebugLogFileName );
@unlink( $xdebugLogFileName );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01949.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" xdebug:ctrl_socket="xdebug-ctrl.%s"><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> detach -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="1" status="stopping" reason="ok"></response>

[%d] Log opened at %s
[%d] [Config] WARN: Due to unavailable TSC clock, setting poll granularity to 100ms instead of 25ms
[%d] [Config] INFO: Control socket set up successfully: '@xdebug-ctrl.%s'
[%d] [Step Debug] INFO: Connecting to configured address/port: %s
[%d] [Step Debug] INFO: Connected to debugging client: %s
[%d] [Step Debug] -> <init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" %s></init>

[%d] [Step Debug] <- detach -i 1
[%d] [Step Debug] -> <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="1" status="stopping" reason="ok"></response>

[%d] Log closed at %s
