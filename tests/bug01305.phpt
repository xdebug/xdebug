--TEST--
Test for bug #1305: Problems with array keys with an aposprophe
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--INI--
xdebug.auto_trace=1
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug01305.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 10',
	'run',
	'property_get -d 0 -c 0 -n $settings',
	'property_get -d 0 -c 0 -n "$settings[\"One\'s Stuff\"]"',
	'property_get -d 0 -c 0 -n "$settings[\"Two\\\\\"s Stuff\"]"',
	'property_get -d 0 -c 0 -n "$settings[\"Three\\\\\\\\s Stuff\"]"',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///tmp/xdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="3"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="10"></xdebug:message></response>

-> property_get -i 4 -d 0 -c 0 -n $settings
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$settings" fullname="$settings" type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="One&#39;s Stuff" fullname="$settings[&quot;One&#39;s Stuff&quot;]" type="array" children="1" numchildren="1"></property><property name="Two&quot;s Stuff" fullname="$settings[&quot;Two\&quot;s Stuff&quot;]" type="array" children="1" numchildren="1"></property><property name="Three\s Stuff" fullname="$settings[&quot;Three\\s Stuff&quot;]" type="array" children="1" numchildren="1"></property></property></response>

-> property_get -i 5 -d 0 -c 0 -n "$settings[\"One's Stuff\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$settings[&quot;One&#39;s Stuff&quot;]" fullname="$settings[&quot;One&#39;s Stuff&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="id" fullname="$settings[&quot;One&#39;s Stuff&quot;][&quot;id&quot;]" type="int"><![CDATA[1000]]></property></property></response>

-> property_get -i 6 -d 0 -c 0 -n "$settings[\"Two\\\"s Stuff\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$settings[&quot;Two\&quot;s Stuff&quot;]" fullname="$settings[&quot;Two\&quot;s Stuff&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="id" fullname="$settings[&quot;Two\&quot;s Stuff&quot;][&quot;id&quot;]" type="int"><![CDATA[2000]]></property></property></response>

-> property_get -i 7 -d 0 -c 0 -n "$settings[\"Three\\\\s Stuff\"]"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$settings[&quot;Three\\s Stuff&quot;]" fullname="$settings[&quot;Three\\s Stuff&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="id" fullname="$settings[&quot;Three\\s Stuff&quot;][&quot;id&quot;]" type="int"><![CDATA[3000]]></property></property></response>
