--TEST--
Starting Debugger: overridden POST ignore value is 'no'
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$xdebugLogFileName = getTmpFile('start_ignore_no_env.txt');
@unlink( $xdebugLogFileName );

dbgpRunFile(
	dirname(__FILE__) . '/break-echo.inc',
	['stack_get', 'step_into', 'detach'],
	[
		'xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'yes',
		'xdebug.log' => $xdebugLogFileName, 'xdebug.log_level' => 10,
	],
	['timeout' => 1, 'env' => [ 'XDEBUG_IGNORE' => 'yes' ], 'auto_prepend' => '<?php $_POST["XDEBUG_IGNORE"] = "no";']
);

echo file_get_contents( $xdebugLogFileName );
@unlink( $xdebugLogFileName );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://%Sauto-prepend.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> stack_get -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="1"></response>

-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://break-echo.inc" lineno="2"></xdebug:message></response>

-> detach -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="3" status="stopping" reason="ok"></response>

[%d] Log opened at %s%A
[%d] [Step Debug] DEBUG: Not activating because an 'XDEBUG_IGNORE' ENV variable is present, with value 'yes'.
[%d] [Step Debug] INFO: Not ignoring present 'XDEBUG_IGNORE' POST variable, because the value is 'no'.
%A
[%d] Log closed at %s
