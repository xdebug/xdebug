--TEST--
Test for bug #1305: Problems with array keys with an aposprophe
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01305.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 11',
	'run',
	'property_get -d 0 -c 0 -n $settings',
	'property_get -d 0 -c 0 -n "$settings[\"One\'s Stuff\"]"',
	'property_get -d 0 -c 0 -n "$settings[\"One\\\'s Stuff\"]"',
	'property_get -d 0 -c 0 -n "$settings[\"Two\\\\\"s Stuff\"]"',
	'property_get -d 0 -c 0 -n "$settings[\"Three\\\\\\\\s Stuff\"]"',
	'property_get -d 0 -c 0 -n $settings["Four\'sStuff"]',
	'property_get -d 0 -c 0 -n "$settings[\"Four\'sStuff\"]"',
	'property_get -d 0 -c 0 -n "$settings[\"Five\'sSt\\\0ff\"]"',
	'feature_set -n extended_properties -v 1',
	'property_get -d 0 -c 0 -n $settings',
	'property_get -d 0 -c 0 -n "$settings[\"One\'s Stuff\"]"',
	'property_get -d 0 -c 0 -n "$settings[\"One\\\'s Stuff\"]"',
	'property_get -d 0 -c 0 -n "$settings[\"Five\'sSt\\\0ff\"]"',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01305.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01305.inc" lineno="3"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug01305.inc" lineno="11"></xdebug:message></response>

-> property_get -i 4 -d 0 -c 0 -n $settings
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$settings" fullname="$settings" type="array" children="1" numchildren="5" page="0" pagesize="32"><property name="One&#39;s Stuff" fullname="$settings[&quot;One\&#39;s Stuff&quot;]" type="array" children="1" numchildren="1"></property><property name="Two&quot;s Stuff" fullname="$settings[&quot;Two\&quot;s Stuff&quot;]" type="array" children="1" numchildren="1"></property><property name="Three\s Stuff" fullname="$settings[&quot;Three\\s Stuff&quot;]" type="array" children="1" numchildren="1"></property><property name="Four&#39;sStuff" fullname="$settings[&quot;Four\&#39;sStuff&quot;]" type="array" children="1" numchildren="1"></property><property name="Five&#39;sSt&#0;ff" fullname="$settings[&quot;Five\&#39;sSt\0ff&quot;]" type="array" children="1" numchildren="1"></property></property></response>

-> property_get -i 5 -d 0 -c 0 -n "$settings[\"One's Stuff\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$settings[&quot;One&#39;s Stuff&quot;]" fullname="$settings[&quot;One&#39;s Stuff&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="id" fullname="$settings[&quot;One&#39;s Stuff&quot;][&quot;id&quot;]" type="int"><![CDATA[1000]]></property></property></response>

-> property_get -i 6 -d 0 -c 0 -n "$settings[\"One\'s Stuff\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$settings[&quot;One&#39;s Stuff&quot;]" fullname="$settings[&quot;One&#39;s Stuff&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="id" fullname="$settings[&quot;One&#39;s Stuff&quot;][&quot;id&quot;]" type="int"><![CDATA[1000]]></property></property></response>

-> property_get -i 7 -d 0 -c 0 -n "$settings[\"Two\\\"s Stuff\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$settings[&quot;Two\&quot;s Stuff&quot;]" fullname="$settings[&quot;Two\&quot;s Stuff&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="id" fullname="$settings[&quot;Two\&quot;s Stuff&quot;][&quot;id&quot;]" type="int"><![CDATA[2000]]></property></property></response>

-> property_get -i 8 -d 0 -c 0 -n "$settings[\"Three\\\\s Stuff\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$settings[&quot;Three\\s Stuff&quot;]" fullname="$settings[&quot;Three\\s Stuff&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="id" fullname="$settings[&quot;Three\\s Stuff&quot;][&quot;id&quot;]" type="int"><![CDATA[3000]]></property></property></response>

-> property_get -i 9 -d 0 -c 0 -n $settings["Four'sStuff"]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="$settings[&quot;Four&#39;sStuff&quot;]" fullname="$settings[&quot;Four&#39;sStuff&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="id" fullname="$settings[&quot;Four&#39;sStuff&quot;][&quot;id&quot;]" type="int"><![CDATA[4000]]></property></property></response>

-> property_get -i 10 -d 0 -c 0 -n "$settings[\"Four'sStuff\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="10"><property name="$settings[&quot;Four&#39;sStuff&quot;]" fullname="$settings[&quot;Four&#39;sStuff&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="id" fullname="$settings[&quot;Four&#39;sStuff&quot;][&quot;id&quot;]" type="int"><![CDATA[4000]]></property></property></response>

-> property_get -i 11 -d 0 -c 0 -n "$settings[\"Five'sSt\\0ff\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="11"><property name="$settings[&quot;Five&#39;sSt\0ff&quot;]" fullname="$settings[&quot;Five&#39;sSt\0ff&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="id" fullname="$settings[&quot;Five&#39;sSt\0ff&quot;][&quot;id&quot;]" type="int"><![CDATA[5000]]></property></property></response>

-> feature_set -i 12 -n extended_properties -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="12" feature="extended_properties" success="1"></response>

-> property_get -i 13 -d 0 -c 0 -n $settings
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="13"><property name="$settings" fullname="$settings" type="array" children="1" numchildren="5" page="0" pagesize="32"><property name="One&#39;s Stuff" fullname="$settings[&quot;One\&#39;s Stuff&quot;]" type="array" children="1" numchildren="1"></property><property name="Two&quot;s Stuff" fullname="$settings[&quot;Two\&quot;s Stuff&quot;]" type="array" children="1" numchildren="1"></property><property name="Three\s Stuff" fullname="$settings[&quot;Three\\s Stuff&quot;]" type="array" children="1" numchildren="1"></property><property name="Four&#39;sStuff" fullname="$settings[&quot;Four\&#39;sStuff&quot;]" type="array" children="1" numchildren="1"></property><property type="array" children="1" numchildren="1"><name encoding="base64"><![CDATA[Rml2ZSdzU3QAZmY=]]></name><fullname encoding="base64"><![CDATA[JHNldHRpbmdzWyJGaXZlXCdzU3RcMGZmIl0=]]></fullname></property></property></response>

-> property_get -i 14 -d 0 -c 0 -n "$settings[\"One's Stuff\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="14"><property name="$settings[&quot;One&#39;s Stuff&quot;]" fullname="$settings[&quot;One&#39;s Stuff&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="id" fullname="$settings[&quot;One&#39;s Stuff&quot;][&quot;id&quot;]" type="int"><![CDATA[1000]]></property></property></response>

-> property_get -i 15 -d 0 -c 0 -n "$settings[\"One\'s Stuff\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="15"><property name="$settings[&quot;One&#39;s Stuff&quot;]" fullname="$settings[&quot;One&#39;s Stuff&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="id" fullname="$settings[&quot;One&#39;s Stuff&quot;][&quot;id&quot;]" type="int"><![CDATA[1000]]></property></property></response>

-> property_get -i 16 -d 0 -c 0 -n "$settings[\"Five'sSt\\0ff\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="16"><property name="$settings[&quot;Five&#39;sSt\0ff&quot;]" fullname="$settings[&quot;Five&#39;sSt\0ff&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="id" fullname="$settings[&quot;Five&#39;sSt\0ff&quot;][&quot;id&quot;]" type="int"><![CDATA[5000]]></property></property></response>
