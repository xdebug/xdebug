--TEST--
Test for bug #842: Can't debug conditional statements without a block (TS1a)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug00842.inc' );

$commands = array(
	'feature_set -n notify_ok -v 1',
	'feature_set -n resolved_breakpoints -v 1',
	"breakpoint_set -t line -f file://{$filename} -n 14",
	"breakpoint_set -t line -f file://{$filename} -n 17",
	'run',
	'stack_get',
	'step_into',
	'step_into',
	'stack_get',
	'step_into',
	'step_into',
	'stack_get',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00842.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n notify_ok -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="notify_ok" success="1"></response>

-> feature_set -i 2 -n resolved_breakpoints -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="resolved_breakpoints" success="1"></response>

-> breakpoint_set -i 3 -t line -f file://bug00842.inc -n 14
<?xml version="1.0" encoding="iso-8859-1"?>
<notify xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" name="breakpoint_resolved"><breakpoint type="line" resolved="resolved" filename="file://bug00842.inc" lineno="14" state="enabled" hit_count="0" hit_value="0" id="{{PID}}0001"></breakpoint></notify>

<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="{{PID}}0001" resolved="resolved"></response>

-> breakpoint_set -i 4 -t line -f file://bug00842.inc -n 17
<?xml version="1.0" encoding="iso-8859-1"?>
<notify xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" name="breakpoint_resolved"><breakpoint type="line" resolved="resolved" filename="file://bug00842.inc" lineno="17" state="enabled" hit_count="0" hit_value="0" id="{{PID}}0002"></breakpoint></notify>

<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="4" id="{{PID}}0002" resolved="resolved"></response>

-> run -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file://bug00842.inc" lineno="14"></xdebug:message></response>

-> stack_get -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="6"><stack where="{main}" level="0" type="file" filename="file://bug00842.inc" lineno="14"></stack></response>

-> step_into -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="7" status="break" reason="ok"><xdebug:message filename="file://bug00842.inc" lineno="17"></xdebug:message></response>

-> step_into -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="8" status="break" reason="ok"><xdebug:message filename="file://bug00842.inc" lineno="12"></xdebug:message></response>

-> stack_get -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="9"><stack where="dummy2" level="0" type="file" filename="file://bug00842.inc" lineno="12"></stack><stack where="{main}" level="1" type="file" filename="file://bug00842.inc" lineno="17"></stack></response>

-> step_into -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="10" status="break" reason="ok"><xdebug:message filename="file://bug00842.inc" lineno="13"></xdebug:message></response>

-> step_into -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="11" status="break" reason="ok"><xdebug:message filename="file://bug00842.inc" lineno="18"></xdebug:message></response>

-> stack_get -i 12
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="12"><stack where="{main}" level="0" type="file" filename="file://bug00842.inc" lineno="18"></stack></response>

-> detach -i 13
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="13" status="stopping" reason="ok"></response>
