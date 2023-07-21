--TEST--
Test for bug #978: Inspection of array with negative keys fails
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug00978.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 8',
	'run',
	'property_get -n $a',
	'property_get -n $a[-1]',
	'property_get -n $a[-]',
	'property_get -n $a[1]',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00978.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug00978.inc" lineno="%d"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug00978.inc" lineno="8"></xdebug:message></response>

-> property_get -i 4 -n $a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$a" fullname="$a" type="array" children="1" numchildren="4" page="0" pagesize="32"><property name="0" fullname="$a[0]" type="string" size="3" encoding="base64"><![CDATA[bnVs]]></property><property name="-1" fullname="$a[-1]" type="string" size="9" encoding="base64"><![CDATA[bWludXMgb25l]]></property><property name="-2" fullname="$a[-2]" type="string" size="7" encoding="base64"><![CDATA[bm90IHR3bw==]]></property><property name="1" fullname="$a[1]" type="string" size="3" encoding="base64"><![CDATA[ZWVu]]></property></property></response>

-> property_get -i 5 -n $a[-1]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$a[-1]" fullname="$a[-1]" type="string" size="9" encoding="base64"><![CDATA[bWludXMgb25l]]></property></response>

-> property_get -i 6 -n $a[-]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6" status="break" reason="ok"><error code="300"><message><![CDATA[can not get property]]></message></error></response>

-> property_get -i 7 -n $a[1]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$a[1]" fullname="$a[1]" type="string" size="3" encoding="base64"><![CDATA[ZWVu]]></property></response>

-> detach -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="8" status="stopping" reason="ok"></response>
