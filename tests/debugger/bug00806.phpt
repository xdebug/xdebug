--TEST--
Test for bug #806: 'property_get' doesn't work if array key contains a whitespace
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug00806.inc';

$commands = array(
	'step_into',
	'step_into',
	'context_get',
	"property_get -n \$x['a b'] -d 0 -c 0 -p 0",
	'property_get -n "$x[\'a b\']" -d 0 -c 0 -p 0',
	'property_get -n "$x[\"a b\"]" -d 0 -c 0 -p 0',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00806.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug00806.inc" lineno="2"></xdebug:message></response>

-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug00806.inc" lineno="3"></xdebug:message></response>

-> context_get -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="3" context="0"><property name="$x" fullname="$x" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="a" fullname="$x[&quot;a&quot;]" type="array" children="1" numchildren="2"></property><property name="a b" fullname="$x[&quot;a b&quot;]" type="array" children="1" numchildren="2"></property></property></response>

-> property_get -i 4 -n $x['a b'] -d 0 -c 0 -p 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><error code="1"><message><![CDATA[parse error in command]]></message></error></response>

-> property_get -i 5 -n "$x['a b']" -d 0 -c 0 -p 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$x[&#39;a b&#39;]" fullname="$x[&#39;a b&#39;]" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="0" fullname="$x[&#39;a b&#39;][0]" type="int"><![CDATA[1]]></property><property name="1" fullname="$x[&#39;a b&#39;][1]" type="int"><![CDATA[2]]></property></property></response>

-> property_get -i 6 -n "$x[\"a b\"]" -d 0 -c 0 -p 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$x[&quot;a b&quot;]" fullname="$x[&quot;a b&quot;]" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="0" fullname="$x[&quot;a b&quot;][0]" type="int"><![CDATA[1]]></property><property name="1" fullname="$x[&quot;a b&quot;][1]" type="int"><![CDATA[2]]></property></property></response>

-> detach -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="7" status="stopping" reason="ok"></response>
