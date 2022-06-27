--TEST--
Test for bug #2098: With breakpoint_include_return_value enabled step_out break at every function (complex) (>= PHP 8.0)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.0; dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02098-002.inc';

$commands = array(
	'feature_set -n breakpoint_include_return_value -v 1',
	'step_into',
	'breakpoint_set -t line -n 10',
	'breakpoint_set -t line -n 20 -r 1',
	'run',
	'step_out',
	'context_get',
	'run',
	'step_out',
	'context_get',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02098-002.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n breakpoint_include_return_value -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="breakpoint_include_return_value" success="1"></response>

-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug02098-002.inc" lineno="29"></xdebug:message></response>

-> breakpoint_set -i 3 -t line -n 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="{{PID}}0001"></response>

-> breakpoint_set -i 4 -t line -n 20 -r 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="4" id="{{PID}}0002"></response>

-> run -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file://bug02098-002.inc" lineno="20"></xdebug:message></response>

-> step_out -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_out" transaction_id="6" status="break" reason="ok"><xdebug:message filename="file://bug02098-002.inc" lineno="30"></xdebug:message><xdebug:return_value><property type="object" classname="Fluent" children="1" numchildren="1" page="0" pagesize="32"><property name="start" facet="private" type="string" size="45" encoding="base64"><![CDATA[UmV0dXJuIHZhbHVlIGluc3BlY3Rpb24gKHBocD04LjEpIChwaHA9Z3JlYXQp]]></property></property></xdebug:return_value></response>

-> context_get -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="7" context="0"><property name="$__RETURN_VALUE" fullname="$__RETURN_VALUE" type="object" classname="Fluent" children="1" numchildren="1" page="0" pagesize="32" facet="readonly return_value virtual"><property name="start" fullname="$__RETURN_VALUE-&gt;start" facet="private" type="string" size="45" encoding="base64"><![CDATA[UmV0dXJuIHZhbHVlIGluc3BlY3Rpb24gKHBocD04LjEpIChwaHA9Z3JlYXQp]]></property></property></response>

-> run -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="8" status="break" reason="ok"><xdebug:message filename="file://bug02098-002.inc" lineno="10"></xdebug:message></response>

-> step_out -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_out" transaction_id="9" status="break" reason="ok"><xdebug:message filename="file://bug02098-002.inc" lineno="32"></xdebug:message><xdebug:return_value><property type="string" size="79" encoding="base64"><![CDATA[UmV0dXJuIHZhbHVlIGluc3BlY3Rpb24gKHBocD04LjEpIChwaHA9Z3JlYXQpICh4ZGVidWc9YXdlc29tZSkgKHhkZWJ1Zz1hbWF6aW5nKQ==]]></property></xdebug:return_value></response>

-> context_get -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="10" context="0"><property name="$__RETURN_VALUE" fullname="$__RETURN_VALUE" type="string" size="79" facet="readonly return_value virtual" encoding="base64"><![CDATA[UmV0dXJuIHZhbHVlIGluc3BlY3Rpb24gKHBocD04LjEpIChwaHA9Z3JlYXQpICh4ZGVidWc9YXdlc29tZSkgKHhkZWJ1Zz1hbWF6aW5nKQ==]]></property></response>

-> detach -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="11" status="stopping" reason="ok"></response>
