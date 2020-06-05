--TEST--
Test for bug #1520: Xdebug does not handle variable names with "-" in their name
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01520.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 19',
	'run',
	'property_get -n $obj',
	'property_get -n $obj->{"with-dash-char"}',
	'property_get -n $obj->{"with[\\\'square\\\']"}',
	'property_get -n $obj->{"{with"}',
	'property_get -n $obj->{"{wi\\"th"}',
	'property_get -n $obj->{"two[\\\'square\\\']"}',
	'property_get -n $obj->{"two[\\\'square\\\']"}["{with"]',
	'property_get -n $obj->{"two[\\\'square\\\']"}["{wi\\"th"]',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01520.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01520.inc" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 19
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug01520.inc" lineno="19"></xdebug:message></response>

-> property_get -i 4 -n $obj
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$obj" fullname="$obj" type="object" classname="stdClass" children="1" numchildren="5" page="0" pagesize="32"><property name="with-dash-char" fullname="$obj-&gt;{&quot;with-dash-char&quot;}" facet="public" type="int"><![CDATA[42]]></property><property name="with[&#39;square&#39;]" fullname="$obj-&gt;{&quot;with[\&#39;square\&#39;]&quot;}" facet="public" type="int"><![CDATA[43]]></property><property name="{with" fullname="$obj-&gt;{&quot;{with&quot;}" facet="public" type="int"><![CDATA[45]]></property><property name="{wi&quot;th" fullname="$obj-&gt;{&quot;{wi\&quot;th&quot;}" facet="public" type="int"><![CDATA[46]]></property><property name="two[&#39;square&#39;]" fullname="$obj-&gt;{&quot;two[\&#39;square\&#39;]&quot;}" facet="public" type="array" children="1" numchildren="2"></property></property></response>

-> property_get -i 5 -n $obj->{"with-dash-char"}
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$obj-&gt;{&quot;with-dash-char&quot;}" fullname="$obj-&gt;{&quot;with-dash-char&quot;}" type="int"><![CDATA[42]]></property></response>

-> property_get -i 6 -n $obj->{"with[\'square\']"}
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$obj-&gt;{&quot;with[\&#39;square\&#39;]&quot;}" fullname="$obj-&gt;{&quot;with[\&#39;square\&#39;]&quot;}" type="int"><![CDATA[43]]></property></response>

-> property_get -i 7 -n $obj->{"{with"}
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$obj-&gt;{&quot;{with&quot;}" fullname="$obj-&gt;{&quot;{with&quot;}" type="int"><![CDATA[45]]></property></response>

-> property_get -i 8 -n $obj->{"{wi\"th"}
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$obj-&gt;{&quot;{wi\&quot;th&quot;}" fullname="$obj-&gt;{&quot;{wi\&quot;th&quot;}" type="int"><![CDATA[46]]></property></response>

-> property_get -i 9 -n $obj->{"two[\'square\']"}
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="$obj-&gt;{&quot;two[\&#39;square\&#39;]&quot;}" fullname="$obj-&gt;{&quot;two[\&#39;square\&#39;]&quot;}" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="{wi&quot;th" fullname="$obj-&gt;{&quot;two[\&#39;square\&#39;]&quot;}[&quot;{wi\&quot;th&quot;]" type="int"><![CDATA[4]]></property><property name="{with" fullname="$obj-&gt;{&quot;two[\&#39;square\&#39;]&quot;}[&quot;{with&quot;]" type="int"><![CDATA[5]]></property></property></response>

-> property_get -i 10 -n $obj->{"two[\'square\']"}["{with"]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="10"><property name="$obj-&gt;{&quot;two[\&#39;square\&#39;]&quot;}[&quot;{with&quot;]" fullname="$obj-&gt;{&quot;two[\&#39;square\&#39;]&quot;}[&quot;{with&quot;]" type="int"><![CDATA[5]]></property></response>

-> property_get -i 11 -n $obj->{"two[\'square\']"}["{wi\"th"]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="11"><property name="$obj-&gt;{&quot;two[\&#39;square\&#39;]&quot;}[&quot;{wi\&quot;th&quot;]" fullname="$obj-&gt;{&quot;two[\&#39;square\&#39;]&quot;}[&quot;{wi\&quot;th&quot;]" type="int"><![CDATA[4]]></property></response>

-> detach -i 12
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="12" status="stopping" reason="ok"></response>
