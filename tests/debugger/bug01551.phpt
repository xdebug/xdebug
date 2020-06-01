--TEST--
Test for bug #1551: Can't debug properties with an empty name
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01551.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 10',
	'run',
	'property_get -n $object',
	'property_get -n $object->{""}',
	'property_get -n $object->{""}[2]',
	'property_get -n $array',
	'property_get -n $array[""]',
	'property_get -n $array[""][2]',
	'detach'
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01551.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01551.inc" lineno="3"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug01551.inc" lineno="10"></xdebug:message></response>

-> property_get -i 4 -n $object
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$object" fullname="$object" type="object" classname="stdClass" children="1" numchildren="1" page="0" pagesize="32"><property name="" fullname="$object-&gt;{&quot;&quot;}" facet="public" type="array" children="1" numchildren="3"></property></property></response>

-> property_get -i 5 -n $object->{""}
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$object-&gt;{&quot;&quot;}" fullname="$object-&gt;{&quot;&quot;}" type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="0" fullname="$object-&gt;{&quot;&quot;}[0]" type="int"><![CDATA[1]]></property><property name="1" fullname="$object-&gt;{&quot;&quot;}[1]" type="int"><![CDATA[2]]></property><property name="2" fullname="$object-&gt;{&quot;&quot;}[2]" type="array" children="1" numchildren="3"></property></property></response>

-> property_get -i 6 -n $object->{""}[2]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$object-&gt;{&quot;&quot;}[2]" fullname="$object-&gt;{&quot;&quot;}[2]" type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="0" fullname="$object-&gt;{&quot;&quot;}[2][0]" type="int"><![CDATA[3]]></property><property name="1" fullname="$object-&gt;{&quot;&quot;}[2][1]" type="int"><![CDATA[4]]></property><property name="2" fullname="$object-&gt;{&quot;&quot;}[2][2]" type="int"><![CDATA[5]]></property></property></response>

-> property_get -i 7 -n $array
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$array" fullname="$array" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="" fullname="$array[&quot;&quot;]" type="array" children="1" numchildren="3"></property></property></response>

-> property_get -i 8 -n $array[""]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$array[&quot;&quot;]" fullname="$array[&quot;&quot;]" type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="0" fullname="$array[&quot;&quot;][0]" type="int"><![CDATA[1]]></property><property name="1" fullname="$array[&quot;&quot;][1]" type="int"><![CDATA[2]]></property><property name="2" fullname="$array[&quot;&quot;][2]" type="array" children="1" numchildren="3"></property></property></response>

-> property_get -i 9 -n $array[""][2]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="$array[&quot;&quot;][2]" fullname="$array[&quot;&quot;][2]" type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="0" fullname="$array[&quot;&quot;][2][0]" type="int"><![CDATA[3]]></property><property name="1" fullname="$array[&quot;&quot;][2][1]" type="int"><![CDATA[4]]></property><property name="2" fullname="$array[&quot;&quot;][2][2]" type="int"><![CDATA[5]]></property></property></response>

-> detach -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="10" status="stopping" reason="ok"></response>
