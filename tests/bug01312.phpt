--TEST--
Test for bug #1312: DBGP: extended_properties for \0 characters in fields
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--INI--
xdebug.remote_enable=1
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug01312.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 26',
	'run',
	'property_get -n $clone',
	'property_get -n "$clone[\"\\\0A\\\0testA\"]"',
	'feature_set -n extended_properties -v 1',
	'property_get -n $clone',
	'feature_set -n extended_properties -v 0',
	'property_get -n $clone',
	'detach'
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file:///tmp/xdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="3"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 26
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="26"></xdebug:message></response>

-> property_get -i 4 -n $clone
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$clone" fullname="$clone" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="&#0;A&#0;testA" fullname="$clone[&quot;\0A\0testA&quot;]" type="string" size="8" encoding="base64"><![CDATA[dGVzdGFibGU=]]></property><property name="&#0;A&#0;testC" fullname="$clone[&quot;\0A\0testC&quot;]" type="object" classname="B" children="1" numchildren="1"></property></property></response>

-> property_get -i 5 -n "$clone[\"\\0A\\0testA\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$clone[&quot;\0A\0testA&quot;]" fullname="$clone[&quot;\0A\0testA&quot;]" type="string" size="8" encoding="base64"><![CDATA[dGVzdGFibGU=]]></property></response>

-> feature_set -i 6 -n extended_properties -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="6" feature="extended_properties" success="1"></response>

-> property_get -i 7 -n $clone
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$clone" fullname="$clone" type="array" children="1" numchildren="2" page="0" pagesize="32"><property type="string" size="8"><name encoding="base64"><![CDATA[AEEAdGVzdEE=]]></name><fullname encoding="base64"><![CDATA[JGNsb25lWyJcMEFcMHRlc3RBIl0=]]></fullname><value encoding="base64"><![CDATA[dGVzdGFibGU=]]></value></property><property type="object" children="1" numchildren="1"><name encoding="base64"><![CDATA[AEEAdGVzdEM=]]></name><fullname encoding="base64"><![CDATA[JGNsb25lWyJcMEFcMHRlc3RDIl0=]]></fullname><classname encoding="base64"><![CDATA[Qg==]]></classname></property></property></response>

-> feature_set -i 8 -n extended_properties -v 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="8" feature="extended_properties" success="1"></response>

-> property_get -i 9 -n $clone
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="$clone" fullname="$clone" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="&#0;A&#0;testA" fullname="$clone[&quot;\0A\0testA&quot;]" type="string" size="8" encoding="base64"><![CDATA[dGVzdGFibGU=]]></property><property name="&#0;A&#0;testC" fullname="$clone[&quot;\0A\0testC&quot;]" type="object" classname="B" children="1" numchildren="1"></property></property></response>

-> detach -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="10" status="stopping" reason="ok"></response>
