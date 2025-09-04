--TEST--
Test for bug #2363: Add better debugging support for PHP 8.5's pipes
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.5; dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02363.inc';

$commands = array(
	'step_into',
	'step_over',
	'context_get',
	'step_over',
	'context_get',
	'step_over',
	'context_get',
	'step_over',
	'context_get',
	'step_over',
	'context_get',
	'step_over',
	'context_get',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02363.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug02363.inc" lineno="2"></xdebug:message></response>

-> step_over -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug02363.inc" lineno="3"></xdebug:message></response>

-> context_get -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="3" context="0"><property name="$myString" fullname="$myString" type="string" size="11" encoding="base64"><![CDATA[SGVsbG8gV29ybGQ=]]></property><property name="$result" fullname="$result" type="uninitialized"></property></response>

-> step_over -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://bug02363.inc" lineno="6"></xdebug:message></response>

-> context_get -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="5" context="0"><property name="$__INTERMEDIATE_VALUE" fullname="$__INTERMEDIATE_VALUE" type="string" size="13" facet="readonly intermediate_value virtual" encoding="base64"><![CDATA[PEhlbGxvIFdvcmxkPg==]]></property><property name="$myString" fullname="$myString" type="string" size="11" encoding="base64"><![CDATA[SGVsbG8gV29ybGQ=]]></property><property name="$result" fullname="$result" type="uninitialized"></property></response>

-> step_over -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="6" status="break" reason="ok"><xdebug:message filename="file://bug02363.inc" lineno="10"></xdebug:message></response>

-> context_get -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="7" context="0"><property name="$__INTERMEDIATE_VALUE" fullname="$__INTERMEDIATE_VALUE" type="string" size="19" facet="readonly intermediate_value virtual" encoding="base64"><![CDATA[Jmx0O0hlbGxvIFdvcmxkJmd0Ow==]]></property><property name="$myString" fullname="$myString" type="string" size="11" encoding="base64"><![CDATA[SGVsbG8gV29ybGQ=]]></property><property name="$result" fullname="$result" type="uninitialized"></property></response>

-> step_over -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="8" status="break" reason="ok"><xdebug:message filename="file://bug02363.inc" lineno="12"></xdebug:message></response>

-> context_get -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="9" context="0"><property name="$__INTERMEDIATE_VALUE" fullname="$__INTERMEDIATE_VALUE" type="array" children="1" numchildren="19" page="0" pagesize="32" facet="readonly intermediate_value virtual"><property name="0" fullname="$__INTERMEDIATE_VALUE[0]" type="string" size="1" encoding="base64"><![CDATA[Jg==]]></property><property name="1" fullname="$__INTERMEDIATE_VALUE[1]" type="string" size="1" encoding="base64"><![CDATA[bA==]]></property><property name="2" fullname="$__INTERMEDIATE_VALUE[2]" type="string" size="1" encoding="base64"><![CDATA[dA==]]></property><property name="3" fullname="$__INTERMEDIATE_VALUE[3]" type="string" size="1" encoding="base64"><![CDATA[Ow==]]></property><property name="4" fullname="$__INTERMEDIATE_VALUE[4]" type="string" size="1" encoding="base64"><![CDATA[SA==]]></property><property name="5" fullname="$__INTERMEDIATE_VALUE[5]" type="string" size="1" encoding="base64"><![CDATA[ZQ==]]></property><property name="6" fullname="$__INTERMEDIATE_VALUE[6]" type="string" size="1" encoding="base64"><![CDATA[bA==]]></property><property name="7" fullname="$__INTERMEDIATE_VALUE[7]" type="string" size="1" encoding="base64"><![CDATA[bA==]]></property><property name="8" fullname="$__INTERMEDIATE_VALUE[8]" type="string" size="1" encoding="base64"><![CDATA[bw==]]></property><property name="9" fullname="$__INTERMEDIATE_VALUE[9]" type="string" size="1" encoding="base64"><![CDATA[IA==]]></property><property name="10" fullname="$__INTERMEDIATE_VALUE[10]" type="string" size="1" encoding="base64"><![CDATA[Vw==]]></property><property name="11" fullname="$__INTERMEDIATE_VALUE[11]" type="string" size="1" encoding="base64"><![CDATA[bw==]]></property><property name="12" fullname="$__INTERMEDIATE_VALUE[12]" type="string" size="1" encoding="base64"><![CDATA[cg==]]></property><property name="13" fullname="$__INTERMEDIATE_VALUE[13]" type="string" size="1" encoding="base64"><![CDATA[bA==]]></property><property name="14" fullname="$__INTERMEDIATE_VALUE[14]" type="string" size="1" encoding="base64"><![CDATA[ZA==]]></property><property name="15" fullname="$__INTERMEDIATE_VALUE[15]" type="string" size="1" encoding="base64"><![CDATA[Jg==]]></property><property name="16" fullname="$__INTERMEDIATE_VALUE[16]" type="string" size="1" encoding="base64"><![CDATA[Zw==]]></property><property name="17" fullname="$__INTERMEDIATE_VALUE[17]" type="string" size="1" encoding="base64"><![CDATA[dA==]]></property><property name="18" fullname="$__INTERMEDIATE_VALUE[18]" type="string" size="1" encoding="base64"><![CDATA[Ow==]]></property></property><property name="$myString" fullname="$myString" type="string" size="11" encoding="base64"><![CDATA[SGVsbG8gV29ybGQ=]]></property><property name="$result" fullname="$result" type="uninitialized"></property></response>

-> step_over -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="10" status="break" reason="ok"><xdebug:message filename="file://bug02363.inc" lineno="14"></xdebug:message></response>

-> context_get -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="11" context="0"><property name="$__INTERMEDIATE_VALUE" fullname="$__INTERMEDIATE_VALUE" type="array" children="1" numchildren="19" page="0" pagesize="32" facet="readonly intermediate_value virtual"><property name="0" fullname="$__INTERMEDIATE_VALUE[0]" type="string" size="1" encoding="base64"><![CDATA[Jg==]]></property><property name="1" fullname="$__INTERMEDIATE_VALUE[1]" type="string" size="1" encoding="base64"><![CDATA[TA==]]></property><property name="2" fullname="$__INTERMEDIATE_VALUE[2]" type="string" size="1" encoding="base64"><![CDATA[VA==]]></property><property name="3" fullname="$__INTERMEDIATE_VALUE[3]" type="string" size="1" encoding="base64"><![CDATA[Ow==]]></property><property name="4" fullname="$__INTERMEDIATE_VALUE[4]" type="string" size="1" encoding="base64"><![CDATA[SA==]]></property><property name="5" fullname="$__INTERMEDIATE_VALUE[5]" type="string" size="1" encoding="base64"><![CDATA[RQ==]]></property><property name="6" fullname="$__INTERMEDIATE_VALUE[6]" type="string" size="1" encoding="base64"><![CDATA[TA==]]></property><property name="7" fullname="$__INTERMEDIATE_VALUE[7]" type="string" size="1" encoding="base64"><![CDATA[TA==]]></property><property name="8" fullname="$__INTERMEDIATE_VALUE[8]" type="string" size="1" encoding="base64"><![CDATA[Tw==]]></property><property name="9" fullname="$__INTERMEDIATE_VALUE[9]" type="string" size="1" encoding="base64"><![CDATA[IA==]]></property><property name="10" fullname="$__INTERMEDIATE_VALUE[10]" type="string" size="1" encoding="base64"><![CDATA[Vw==]]></property><property name="11" fullname="$__INTERMEDIATE_VALUE[11]" type="string" size="1" encoding="base64"><![CDATA[Tw==]]></property><property name="12" fullname="$__INTERMEDIATE_VALUE[12]" type="string" size="1" encoding="base64"><![CDATA[Ug==]]></property><property name="13" fullname="$__INTERMEDIATE_VALUE[13]" type="string" size="1" encoding="base64"><![CDATA[TA==]]></property><property name="14" fullname="$__INTERMEDIATE_VALUE[14]" type="string" size="1" encoding="base64"><![CDATA[RA==]]></property><property name="15" fullname="$__INTERMEDIATE_VALUE[15]" type="string" size="1" encoding="base64"><![CDATA[Jg==]]></property><property name="16" fullname="$__INTERMEDIATE_VALUE[16]" type="string" size="1" encoding="base64"><![CDATA[Rw==]]></property><property name="17" fullname="$__INTERMEDIATE_VALUE[17]" type="string" size="1" encoding="base64"><![CDATA[VA==]]></property><property name="18" fullname="$__INTERMEDIATE_VALUE[18]" type="string" size="1" encoding="base64"><![CDATA[Ow==]]></property></property><property name="$myString" fullname="$myString" type="string" size="11" encoding="base64"><![CDATA[SGVsbG8gV29ybGQ=]]></property><property name="$result" fullname="$result" type="uninitialized"></property></response>

-> step_over -i 12
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="12" status="break" reason="ok"><xdebug:message filename="file://bug02363.inc" lineno="16"></xdebug:message></response>

-> context_get -i 13
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="13" context="0"><property name="$myString" fullname="$myString" type="string" size="11" encoding="base64"><![CDATA[SGVsbG8gV29ybGQ=]]></property><property name="$result" fullname="$result" type="string" size="55" encoding="base64"><![CDATA[JiwgTCwgVCwgOywgSCwgRSwgTCwgTCwgTywgICwgVywgTywgUiwgTCwgRCwgJiwgRywgVCwgOw==]]></property></response>

-> detach -i 14
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="14" status="stopping" reason="ok"></response>
