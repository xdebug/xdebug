--TEST--
Test for bug #606: evaluate a $this->... expression when $this is not accesible crash xdebug
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug00606-002.inc';

$commands = array(
	'breakpoint_set -t exception -x Warning',
	'breakpoint_list',
	'run',
	'stack_get',
	'eval -- JHRoaXMtPnByb3BlcnR5',
	'stack_get',
	'step_into',
	'stack_get',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00606-002.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t exception -x Warning
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="{{PID}}0001"></response>

-> breakpoint_list -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_list" transaction_id="2"><breakpoint type="exception" exception="Warning" state="enabled" hit_count="0" hit_value="0" id="{{PID}}0001"></breakpoint></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug00606-002.inc" lineno="6" exception="Warning" code="2"><![CDATA[%s]]></xdebug:message></response>

-> stack_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="4"><stack where="hex2bin" level="0" type="file" filename="file://bug00606-002.inc" lineno="6"></stack><stack where="test" level="1" type="file" filename="file://bug00606-002.inc" lineno="6"></stack><stack where="{main}" level="2" type="file" filename="file://bug00606-002.inc" lineno="8"></stack></response>

-> eval -i 5 -- JHRoaXMtPnByb3BlcnR5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="5" status="break" reason="ok"><error code="206"><message><![CDATA[error evaluating code: %s]]></message></error></response>

-> stack_get -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="6"><stack where="hex2bin" level="0" type="file" filename="file://bug00606-002.inc" lineno="6"></stack><stack where="test" level="1" type="file" filename="file://bug00606-002.inc" lineno="6"></stack><stack where="{main}" level="2" type="file" filename="file://bug00606-002.inc" lineno="8"></stack></response>

-> step_into -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="7" status="break" reason="ok"><xdebug:message filename="file://bug00606-002.inc" lineno="7"></xdebug:message></response>

-> stack_get -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="8"><stack where="test" level="0" type="file" filename="file://bug00606-002.inc" lineno="7"></stack><stack where="{main}" level="1" type="file" filename="file://bug00606-002.inc" lineno="8"></stack></response>

-> detach -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="9" status="stopping" reason="ok"></response>
