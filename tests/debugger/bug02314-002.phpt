--TEST--
Test for bug #2314: Class properties with hooks are always shown as null [2]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.4; dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02314-002.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 34',
	'run',
	'context_get -c 0',
	'property_get -c 0 -n $foo->date',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02314-002.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug02314-002.inc" lineno="%d"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 34
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug02314-002.inc" lineno="34"></xdebug:message></response>

-> context_get -i 4 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property name="$date" fullname="$date" type="object" classname="DateTime" children="1" numchildren="3" page="0" pagesize="32"><property name="date" fullname="$date-&gt;date" facet="public" type="string" size="26" encoding="base64"><![CDATA[MTk4MC0wMS0wMSAwMDowMDowMC4wMDAwMDA=]]></property><property name="timezone_type" fullname="$date-&gt;timezone_type" facet="public" type="int"><![CDATA[3]]></property><property name="timezone" fullname="$date-&gt;timezone" facet="public" type="string" size="3" encoding="base64"><![CDATA[VVRD]]></property></property><property name="$foo" fullname="$foo" type="object" classname="Foo" children="1" numchildren="1" page="0" pagesize="32"><property name="date" fullname="$foo-&gt;date" facet="private" type="object" classname="DateTime" children="1" numchildren="3"></property></property></response>

-> property_get -i 5 -c 0 -n $foo->date
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$foo-&gt;date" fullname="$foo-&gt;date" type="object" classname="DateTime" children="1" numchildren="3" page="0" pagesize="32"><property name="date" fullname="$foo-&gt;date-&gt;date" facet="public" type="string" size="26" encoding="base64"><![CDATA[MTk3MC0wMS0wMSAxMjozNDo1Ni4wMDAwMDA=]]></property><property name="timezone_type" fullname="$foo-&gt;date-&gt;timezone_type" facet="public" type="int"><![CDATA[3]]></property><property name="timezone" fullname="$foo-&gt;date-&gt;timezone" facet="public" type="string" size="3" encoding="base64"><![CDATA[VVRD]]></property></property></response>

-> detach -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="6" status="stopping" reason="ok"></response>
