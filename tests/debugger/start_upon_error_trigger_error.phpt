--TEST--
Starting Debugger: trigger, error, start_up_error=yes
--FILE--
<?php
require 'dbgp/dbgpclient.php';

dbgpRunFile(
	dirname(__FILE__) . '/break-with-error.inc',
	['stack_get', 'step_into', 'detach'],
	['xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'trigger', 'xdebug.start_upon_error' => 'yes'],
	['timeout' => 1]
);
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://break-with-error.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> stack_get -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="1"><stack where="trigger_error" level="0" type="file" filename="file://break-with-error.inc" lineno="3"></stack><stack where="{main}" level="1" type="file" filename="file://break-with-error.inc" lineno="3"></stack></response>

-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://break-with-error.inc" lineno="4"></xdebug:message></response>

-> detach -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="3" status="stopping" reason="ok"></response>
