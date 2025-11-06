--TEST--
Test for bug #2381: Step debug evaluator crashes when using ::class to deference expression
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02381.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 9',
	'run',
	'eval -- ' . base64_encode('$GLOBALS[\'IDE_EVAL_CACHE\'][\'fac1af2a-5e88-436b-87e6-8be95ef78d42\']=(isset($this,$this->a,$this->a[0]::class))?($this->a[0]::class):"IDE_EVAL_ERR"'),
	'context_get',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02381.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug02381.inc" lineno="13"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug02381.inc" lineno="9"></xdebug:message></response>

-> eval -i 4 -- JEdMT0JBTFNbJ0lERV9FVkFMX0NBQ0hFJ11bJ2ZhYzFhZjJhLTVlODgtNDM2Yi04N2U2LThiZTk1ZWY3OGQ0MiddPShpc3NldCgkdGhpcywkdGhpcy0+YSwkdGhpcy0+YVswXTo6Y2xhc3MpKT8oJHRoaXMtPmFbMF06OmNsYXNzKToiSURFX0VWQUxfRVJSIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="4" status="break" reason="ok"><error code="206"><message><![CDATA[error evaluating code]]></message></error></response>

-> context_get -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="5" context="0"><property name="$this" fullname="$this" type="object" classname="T" children="1" numchildren="1" page="0" pagesize="32"><property name="a" fullname="$this-&gt;a" facet="public" type="array" children="1" numchildren="1"></property></property></response>

-> detach -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="6" status="stopping" reason="ok"></response>
