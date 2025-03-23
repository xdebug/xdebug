--TEST--
Test for bug #2261: Control socket in init package (Control Socket with TSC)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; ext-flag control-socket; ext-flag tsc; win');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01949.inc';

$commands = array(
	'detach',
);

$xdebugLogFileName = sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'remote-log-2261-001.txt';
@unlink( $xdebugLogFileName );

dbgpRunFile( $filename, $commands, [ 'xdebug.log' => $xdebugLogFileName, 'xdebug.log_level' => 7, 'xdebug.control_socket' => 'time' ] );

echo file_get_contents( $xdebugLogFileName );
@unlink( $xdebugLogFileName );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01949.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" xdebug:ctrl_socket="\\.\pipe\xdebug-ctrl.%s"><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> detach -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="1" status="stopping" reason="ok"></response>

[%d] Log opened at %s
[%d] [Config] INFO: Control socket set up successfully: '\\.\pipe\xdebug-ctrl.%s'
[%d] [Step Debug] INFO: Connecting to configured address/port: %s
[%d] [Step Debug] INFO: Connected to debugging client: %s
[%d] [Step Debug] -> <init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" %s></init>

[%d] [Step Debug] <- detach -i 1
[%d] [Step Debug] -> <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="1" status="stopping" reason="ok"></response>

[%d] Log closed at %s
