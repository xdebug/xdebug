--TEST--
Test for bug #1312: DBGP: extended_properties for \0 characters in fields
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug01312.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 26",
	'run',
	'property_get -n $clone',
	'property_get -n "$clone[\"\\\0A\\\0testA\"]"',
	'feature_set -n extended_properties -v 1',
	'property_get -n $clone',
	'feature_set -n extended_properties -v 0',
	'property_get -n $clone',
	'detach'
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01312.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug01312.inc -n 26
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id=""></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug01312.inc" lineno="26"></xdebug:message></response>

-> property_get -i 3 -n $clone
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="3"><property name="$clone" fullname="$clone" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="&#0;A&#0;testA" fullname="$clone[&quot;\0A\0testA&quot;]" type="string" size="8" encoding="base64"><![CDATA[dGVzdGFibGU=]]></property><property name="&#0;A&#0;testC" fullname="$clone[&quot;\0A\0testC&quot;]" type="object" classname="B" children="1" numchildren="1"></property></property></response>

-> property_get -i 4 -n "$clone[\"\\0A\\0testA\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$clone[&quot;\0A\0testA&quot;]" fullname="$clone[&quot;\0A\0testA&quot;]" type="string" size="8" encoding="base64"><![CDATA[dGVzdGFibGU=]]></property></response>

-> feature_set -i 5 -n extended_properties -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="5" feature="extended_properties" success="1"></response>

-> property_get -i 6 -n $clone
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$clone" fullname="$clone" type="array" children="1" numchildren="2" page="0" pagesize="32"><property type="string" size="8"><name encoding="base64"><![CDATA[AEEAdGVzdEE=]]></name><fullname encoding="base64"><![CDATA[JGNsb25lWyJcMEFcMHRlc3RBIl0=]]></fullname><value encoding="base64"><![CDATA[dGVzdGFibGU=]]></value></property><property type="object" children="1" numchildren="1"><name encoding="base64"><![CDATA[AEEAdGVzdEM=]]></name><fullname encoding="base64"><![CDATA[JGNsb25lWyJcMEFcMHRlc3RDIl0=]]></fullname><classname encoding="base64"><![CDATA[Qg==]]></classname></property></property></response>

-> feature_set -i 7 -n extended_properties -v 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="7" feature="extended_properties" success="1"></response>

-> property_get -i 8 -n $clone
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$clone" fullname="$clone" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="&#0;A&#0;testA" fullname="$clone[&quot;\0A\0testA&quot;]" type="string" size="8" encoding="base64"><![CDATA[dGVzdGFibGU=]]></property><property name="&#0;A&#0;testC" fullname="$clone[&quot;\0A\0testC&quot;]" type="object" classname="B" children="1" numchildren="1"></property></property></response>

-> detach -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="9" status="stopping" reason="ok"></response>
