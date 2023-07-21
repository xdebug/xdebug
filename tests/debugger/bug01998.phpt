--TEST--
Test for bug #1998: Double facet attribute generated for enums that are stored in properties
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; PHP >= 8.1');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$filename = dirname(__FILE__) . '/bug01998.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 22',
	'run',
	'context_get',
	'property_get -n $totalAmount',
	'property_get -n $totalAmount->currency',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01998.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01998.inc" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 22
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug01998.inc" lineno="22"></xdebug:message></response>

-> context_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property name="$totalAmount" fullname="$totalAmount" type="object" classname="TotalAmount" children="1" numchildren="2" page="0" pagesize="32"><property name="amount" fullname="$totalAmount-&gt;amount" facet="private" type="int"><![CDATA[30]]></property><property name="currency" fullname="$totalAmount-&gt;currency" facet="private enum" type="object" classname="Currency" children="1" numchildren="2"></property></property></response>

-> property_get -i 5 -n $totalAmount
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$totalAmount" fullname="$totalAmount" type="object" classname="TotalAmount" children="1" numchildren="2" page="0" pagesize="32"><property name="amount" fullname="$totalAmount-&gt;amount" facet="private" type="int"><![CDATA[30]]></property><property name="currency" fullname="$totalAmount-&gt;currency" facet="private enum" type="object" classname="Currency" children="1" numchildren="2"></property></property></response>

-> property_get -i 6 -n $totalAmount->currency
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$totalAmount-&gt;currency" fullname="$totalAmount-&gt;currency" type="object" facet="enum" classname="Currency" children="1" numchildren="2" page="0" pagesize="32"><property name="name" fullname="$totalAmount-&gt;currency-&gt;name" facet="public readonly" type="string" size="3" encoding="base64"><![CDATA[RVVS]]></property><property name="value" fullname="$totalAmount-&gt;currency-&gt;value" facet="public readonly" type="string" size="3" encoding="base64"><![CDATA[4oKs]]></property></property></response>
