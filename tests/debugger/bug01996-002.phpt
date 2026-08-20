--TEST--
Test for bug #1996: Visualing closures with first class callable syntax (>= PHP 8.2)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.2; dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01996-002.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 17',
	'run',
	'property_get -n $closure_1',
	'property_get -n $closure_2',
	'property_get -n $closure_3',
	'property_get -n $closure_4',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01996-002.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01996-002.inc" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 17
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug01996-002.inc" lineno="17"></xdebug:message></response>

-> property_get -i 4 -n $closure_1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$closure_1" fullname="$closure_1" type="object" facet="closure" classname="Closure" children="1" numchildren="2" page="0" pagesize="32"><property name="function" fullname="$closure_1-&gt;function" facet="public" type="string" size="6" encoding="base64"><![CDATA[c3Vic3Ry]]></property><property name="parameter" fullname="$closure_1-&gt;parameter" facet="public" type="array" children="1" numchildren="3"></property></property></response>

-> property_get -i 5 -n $closure_2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$closure_2" fullname="$closure_2" type="object" facet="closure" classname="Closure" children="1" numchildren="2" page="0" pagesize="32"><property name="function" fullname="$closure_2-&gt;function" facet="public" type="string" size="12" encoding="base64"><![CDATA[dXNlcl9kZWZpbmVk]]></property><property name="parameter" fullname="$closure_2-&gt;parameter" facet="public" type="array" children="1" numchildren="2"></property></property></response>

-> property_get -i 6 -n $closure_3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$closure_3" fullname="$closure_3" type="object" facet="closure" classname="Closure" children="1" numchildren="2" page="0" pagesize="32"><property name="function" fullname="$closure_3-&gt;function" facet="public" type="string" size="35" encoding="base64"><![CDATA[RGF0ZVRpbWVJbW11dGFibGU6OmNyZWF0ZUZyb21Gb3JtYXQ=]]></property><property name="parameter" fullname="$closure_3-&gt;parameter" facet="public" type="array" children="1" numchildren="3"></property></property></response>

-> property_get -i 7 -n $closure_4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$closure_4" fullname="$closure_4" type="object" facet="closure" classname="Closure" children="1" numchildren="3" page="0" pagesize="32"><property name="function" fullname="$closure_4-&gt;function" facet="public" type="string" size="25" encoding="base64"><![CDATA[RGF0ZVRpbWVJbW11dGFibGU6OmZvcm1hdA==]]></property><property name="this" fullname="$closure_4-&gt;this" facet="public" type="object" classname="DateTimeImmutable" children="1" numchildren="3"></property><property name="parameter" fullname="$closure_4-&gt;parameter" facet="public" type="array" children="1" numchildren="1"></property></property></response>

-> detach -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="8" status="stopping" reason="ok"></response>
