--TEST--
Test for bug #2356: Reading properties with get hooks may modify property value
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.4; dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02356.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 28',
	'run',
	'context_get -c 0',
	'context_get -c 0',
	'context_get -c 0',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02356.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug02356.inc" lineno="%d"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 28
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug02356.inc" lineno="28"></xdebug:message></response>

-> context_get -i 4 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property name="$this" fullname="$this" type="object" classname="TEEST" children="1" numchildren="1" page="0" pagesize="32"><property name="AAA" fullname="$this-&gt;AAA" facet="public" type="string" size="0" encoding="base64"><![CDATA[]]></property></property></response>

-> context_get -i 5 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="5" context="0"><property name="$this" fullname="$this" type="object" classname="TEEST" children="1" numchildren="1" page="0" pagesize="32"><property name="AAA" fullname="$this-&gt;AAA" facet="public" type="string" size="0" encoding="base64"><![CDATA[]]></property></property></response>

-> context_get -i 6 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="6" context="0"><property name="$this" fullname="$this" type="object" classname="TEEST" children="1" numchildren="1" page="0" pagesize="32"><property name="AAA" fullname="$this-&gt;AAA" facet="public" type="string" size="0" encoding="base64"><![CDATA[]]></property></property></response>

-> detach -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="7" status="stopping" reason="ok"></response>
