--TEST--
Test for bug #2326: Step debugger finishes if property debugging handler in PHP throws an exception (< PHP 8.4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 8.4; dbgp');
?>
--EXTENSIONS--
dom
xmlreader
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02326.inc';

$commands = array(
	'step_into',
	'stdout -c 1',
	'breakpoint_set -t line -n 12',
	'run',
	'context_get -c 0',
	'step_over',
	'context_get -c 0',
	'step_into',
	'context_get -c 0',
	'step_into',
	'detach',
);

dbgpRunFile( $filename, $commands, [ 'extension' => [ 'dom', 'xmlreader' ] ] );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02326.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug02326.inc" lineno="2"></xdebug:message></response>

-> stdout -i 2 -c 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stdout" transaction_id="2" success="1"></response>

-> breakpoint_set -i 3 -t line -n 12
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="{{PID}}0001"></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://bug02326.inc" lineno="12"></xdebug:message></response>

-> context_get -i 5 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="5" context="0"><property name="$err" fullname="$err" type="uninitialized"></property><property name="$reader" fullname="$reader" type="null"></property></response>

-> step_over -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="6" status="break" reason="ok"><xdebug:message filename="file://bug02326.inc" lineno="13"></xdebug:message></response>

-> context_get -i 7 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="7" context="0"><property name="$err" fullname="$err" type="bool"><![CDATA[1]]></property><property name="$reader" fullname="$reader" type="object" classname="XMLReader" children="0" numchildren="0" page="0" pagesize="32"></property></response>

-> step_into -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="8" status="break" reason="ok"><xdebug:message filename="file://bug02326.inc" lineno="17"></xdebug:message></response>

-> context_get -i 9 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="9" context="0"><property name="$err" fullname="$err" type="bool"><![CDATA[1]]></property><property name="$reader" fullname="$reader" type="object" classname="XMLReader" children="0" numchildren="0" page="0" pagesize="32"></property></response>

-> step_into -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<stream xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" type="stdout" encoding="base64"><![CDATA[TmV4dCBsaW5lMQ==]]></stream>

<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="10" status="break" reason="ok"><xdebug:message filename="file://bug02326.inc" lineno="18"></xdebug:message></response>

-> detach -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="11" status="stopping" reason="ok"></response>
