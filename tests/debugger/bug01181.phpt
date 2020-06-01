--TEST--
Test for bug #1181: Derived class with __get gets called on fetching base class private property
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug01181.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 17",
	'run',
	'property_get -n $test->*BaseClass*private',
	'step_into',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01181.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug01181.inc -n 17
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id=""></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug01181.inc" lineno="17"></xdebug:message></response>

-> property_get -i 3 -n $test->*BaseClass*private
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="3"><property name="$test-&gt;*BaseClass*private" fullname="$test-&gt;*BaseClass*private" type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="0" fullname="$test-&gt;*BaseClass*private[0]" type="string" size="1" encoding="base64"><![CDATA[YQ==]]></property><property name="1" fullname="$test-&gt;*BaseClass*private[1]" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property><property name="2" fullname="$test-&gt;*BaseClass*private[2]" type="string" size="1" encoding="base64"><![CDATA[Yw==]]></property></property></response>

-> step_into -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://bug01181.inc" lineno="22"></xdebug:message></response>
