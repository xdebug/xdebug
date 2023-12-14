--TEST--
DBGP: return value with internal function as top frame
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.4; dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/dbgp-breakpoint-return-function-006.inc';

$commands = array(
	'feature_set -n breakpoint_details -v 1',
	'feature_set -n breakpoint_include_return_value -v 1',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'step_into',
	'context_get',
	'property_get -n $__RETURN_VALUE',
	'property_get -n $__RETURN_VALUE[1]',
	'property_get -n $__RETURN_VALUE[1]["time"]',
	'property_value -n $__RETURN_VALUE',
	'property_value -n $__RETURN_VALUE[1]',
	'property_value -n $__RETURN_VALUE[1]["time"]',
	'detach',
);

dbgpRunFile( $filename, $commands, [ 'xdebug.mode' => 'develop,debug' ] );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://dbgp-breakpoint-return-function-006.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n breakpoint_details -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="breakpoint_details" success="1"></response>

-> feature_set -i 2 -n breakpoint_include_return_value -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="breakpoint_include_return_value" success="1"></response>

-> step_into -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-return-function-006.inc" lineno="12"></xdebug:message></response>

-> step_into -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-return-function-006.inc" lineno="12"></xdebug:message><xdebug:return_value><property type="null"></property></xdebug:return_value></response>

-> step_into -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-return-function-006.inc" lineno="14"></xdebug:message></response>

-> step_into -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="6" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-return-function-006.inc" lineno="9"></xdebug:message></response>

-> step_into -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="7" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-return-function-006.inc" lineno="9"></xdebug:message><xdebug:return_value><property type="null"></property></xdebug:return_value></response>

-> step_into -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="8" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-return-function-006.inc" lineno="4"></xdebug:message></response>

-> step_into -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="9" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-return-function-006.inc" lineno="4"></xdebug:message><xdebug:return_value><property type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="0" type="array" children="1" numchildren="7"></property><property name="1" type="array" children="1" numchildren="6"></property></property></xdebug:return_value></response>

-> context_get -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="10" context="0"><property name="$__RETURN_VALUE" fullname="$__RETURN_VALUE" type="array" children="1" numchildren="2" page="0" pagesize="32" facet="readonly return_value virtual"><property name="0" fullname="$__RETURN_VALUE[0]" type="array" children="1" numchildren="7"></property><property name="1" fullname="$__RETURN_VALUE[1]" type="array" children="1" numchildren="6"></property></property></response>

-> property_get -i 11 -n $__RETURN_VALUE
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="11"><property name="$__RETURN_VALUE" fullname="$__RETURN_VALUE" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="0" fullname="$__RETURN_VALUE[0]" type="array" children="1" numchildren="7"></property><property name="1" fullname="$__RETURN_VALUE[1]" type="array" children="1" numchildren="6"></property></property></response>

-> property_get -i 12 -n $__RETURN_VALUE[1]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="12"><property name="$__RETURN_VALUE[1]" fullname="$__RETURN_VALUE[1]" type="array" children="1" numchildren="6" page="0" pagesize="32"><property name="time" fullname="$__RETURN_VALUE[1][&quot;time&quot;]" type="float"><![CDATA[%s]]></property><property name="memory" fullname="$__RETURN_VALUE[1][&quot;memory&quot;]" type="int"><![CDATA[%s]]></property><property name="function" fullname="$__RETURN_VALUE[1][&quot;function&quot;]" type="string" size="9" encoding="base64"><![CDATA[%s]]></property><property name="file" fullname="$__RETURN_VALUE[1][&quot;file&quot;]" type="string" size="%d" encoding="base64"><![CDATA[%s]]></property><property name="line" fullname="$__RETURN_VALUE[1][&quot;line&quot;]" type="int"><![CDATA[%s]]></property><property name="params" fullname="$__RETURN_VALUE[1][&quot;params&quot;]" type="array" children="0" numchildren="0"></property></property></response>

-> property_get -i 13 -n $__RETURN_VALUE[1]["time"]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="13"><property name="$__RETURN_VALUE[1][&quot;time&quot;]" fullname="$__RETURN_VALUE[1][&quot;time&quot;]" type="float"><![CDATA[%s]]></property></response>

-> property_value -i 14 -n $__RETURN_VALUE
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_value" transaction_id="14" type="array" children="1" numchildren="2"></response>

-> property_value -i 15 -n $__RETURN_VALUE[1]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_value" transaction_id="15" type="array" children="1" numchildren="6"></response>

-> property_value -i 16 -n $__RETURN_VALUE[1]["time"]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_value" transaction_id="16" type="float"><![CDATA[%s]]></response>

-> detach -i 17
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="17" status="stopping" reason="ok"></response>
