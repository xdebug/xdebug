--TEST--
Test for bug #1203: Accessing static property of a class that has no static properties crashes while remote debugging [2]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01202.inc';

$commands = array(
	'feature_set -n extended_properties -v 1',
	'step_into',
	'step_into',
	'context_get',
	'property_get -n $a',
	'property_get -n $a::a',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01202.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n extended_properties -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="extended_properties" success="1"></response>

-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug01202.inc" lineno="2"></xdebug:message></response>

-> step_into -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug01202.inc" lineno="9"></xdebug:message></response>

-> context_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property type="object" children="1" numchildren="1" page="0" pagesize="32"><name encoding="base64"><![CDATA[JGE=]]></name><fullname encoding="base64"><![CDATA[JGE=]]></fullname><classname encoding="base64"><![CDATA[Y2xhc3NAYW5vbnltb3Vz%s]]></classname><property name="a" fullname="$a-&gt;a" facet="public" type="int"><![CDATA[42]]></property></property></response>

-> property_get -i 5 -n $a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property type="object" children="1" numchildren="1" page="0" pagesize="32"><name encoding="base64"><![CDATA[JGE=]]></name><fullname encoding="base64"><![CDATA[JGE=]]></fullname><classname encoding="base64"><![CDATA[Y2xhc3NAYW5vbnltb3Vz%s]]></classname><property name="a" fullname="$a-&gt;a" facet="public" type="int"><![CDATA[42]]></property></property></response>

-> property_get -i 6 -n $a::a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6" status="break" reason="ok"><error code="300"><message><![CDATA[can not get property]]></message></error></response>

-> detach -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="7" status="stopping" reason="ok"></response>
