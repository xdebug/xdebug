--TEST--
Test for bug #1568: Can't debug object properties that have numeric keys
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; ext ds');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01568.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 6',
	'breakpoint_set -t line -n 23',
	'run',
	'property_get -n $vector',
	'property_get -n $vector->0',
	'run',
	'property_get -n $vector',
	'property_get -n $vector->1',
	'property_get -n $vector->2',
	'detach'
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01568.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01568.inc" lineno="4"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> breakpoint_set -i 3 -t line -n 23
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="{{PID}}0002"></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://bug01568.inc" lineno="6"></xdebug:message></response>

-> property_get -i 5 -n $vector
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$vector" fullname="$vector" type="object" classname="Ds\Vector" children="1" numchildren="1" page="0" pagesize="32"><property name="0" fullname="$vector-&gt;0" facet="public" type="array" children="1" numchildren="1"></property></property></response>

-> property_get -i 6 -n $vector->0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$vector-&gt;0" fullname="$vector-&gt;0" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="0" fullname="$vector-&gt;0[0]" type="int"><![CDATA[1]]></property></property></response>

-> run -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="7" status="break" reason="ok"><xdebug:message filename="file://bug01568.inc" lineno="23"></xdebug:message></response>

-> property_get -i 8 -n $vector
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$vector" fullname="$vector" type="object" classname="Ds\Vector" children="1" numchildren="3" page="0" pagesize="32"><property name="0" fullname="$vector-&gt;0" facet="public" type="object" classname="Foo" children="1" numchildren="1"></property><property name="1" fullname="$vector-&gt;1" facet="public" type="object" classname="Foo" children="1" numchildren="1"></property><property name="2" fullname="$vector-&gt;2" facet="public" type="object" classname="Foo" children="1" numchildren="1"></property></property></response>

-> property_get -i 9 -n $vector->1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="$vector-&gt;1" fullname="$vector-&gt;1" type="object" classname="Foo" children="1" numchildren="1" page="0" pagesize="32"><property name="val" fullname="$vector-&gt;1-&gt;val" facet="private" type="int"><![CDATA[2]]></property></property></response>

-> property_get -i 10 -n $vector->2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="10"><property name="$vector-&gt;2" fullname="$vector-&gt;2" type="object" classname="Foo" children="1" numchildren="1" page="0" pagesize="32"><property name="val" fullname="$vector-&gt;2-&gt;val" facet="private" type="int"><![CDATA[3]]></property></property></response>

-> detach -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="11" status="stopping" reason="ok"></response>
