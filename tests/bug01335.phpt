--TEST--
Test for bug #1335: Debugging with PhpStorm sometimes gives "can not get property"
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug01335.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 39',
	'run',
	'context_get',
	'property_get -d 0 -c 0 -n $b',
	'property_get -d 0 -c 0 -n $b->*TestA\\TestB\\TestC\\A*data1',
	'property_get -d 0 -c 0 -n $b->*TestA\\TestB\\TestC\\A*data1->items',
	'property_get -d 0 -c 0 -n "$b->*TestA\\\\TestB\\\\TestC\\\\A*data1"',
	'property_get -d 0 -c 0 -n "$b->*TestA\\\\TestB\\\\TestC\\\\A*data1->items"',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file:///tmp/xdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="5"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 39
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="39"></xdebug:message></response>

-> context_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property name="$b" fullname="$b" type="object" classname="TestA\TestB\TestC\B" children="1" numchildren="2" page="0" pagesize="32"><property name="data2" fullname="$b-&gt;data2" facet="private" type="object" classname="TestA\TestB\TestC\C" children="1" numchildren="1"></property><property name="*TestA\TestB\TestC\A*data1" fullname="$b-&gt;*TestA\TestB\TestC\A*data1" facet="private" type="object" classname="TestA\TestB\TestC\C" children="1" numchildren="1"></property></property><property name="$x" fullname="$x" type="uninitialized"></property></response>

-> property_get -i 5 -d 0 -c 0 -n $b
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$b" fullname="$b" type="object" classname="TestA\TestB\TestC\B" children="1" numchildren="2" page="0" pagesize="32"><property name="data2" fullname="$b-&gt;data2" facet="private" type="object" classname="TestA\TestB\TestC\C" children="1" numchildren="1"></property><property name="*TestA\TestB\TestC\A*data1" fullname="$b-&gt;*TestA\TestB\TestC\A*data1" facet="private" type="object" classname="TestA\TestB\TestC\C" children="1" numchildren="1"></property></property></response>

-> property_get -i 6 -d 0 -c 0 -n $b->*TestA\TestB\TestC\A*data1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$b-&gt;*TestA\TestB\TestC\A*data1" fullname="$b-&gt;*TestA\TestB\TestC\A*data1" type="object" classname="TestA\TestB\TestC\C" children="1" numchildren="1" page="0" pagesize="32"><property name="items" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items" facet="private" type="array" children="1" numchildren="4"></property></property></response>

-> property_get -i 7 -d 0 -c 0 -n $b->*TestA\TestB\TestC\A*data1->items
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items" type="array" children="1" numchildren="4" page="0" pagesize="32"><property name="0" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[0]" type="int"><![CDATA[1]]></property><property name="1" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[1]" type="int"><![CDATA[2]]></property><property name="2" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[2]" type="int"><![CDATA[3]]></property><property name="3" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[3]" type="int"><![CDATA[4]]></property></property></response>

-> property_get -i 8 -d 0 -c 0 -n "$b->*TestA\\TestB\\TestC\\A*data1"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$b-&gt;*TestA\TestB\TestC\A*data1" fullname="$b-&gt;*TestA\TestB\TestC\A*data1" type="object" classname="TestA\TestB\TestC\C" children="1" numchildren="1" page="0" pagesize="32"><property name="items" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items" facet="private" type="array" children="1" numchildren="4"></property></property></response>

-> property_get -i 9 -d 0 -c 0 -n "$b->*TestA\\TestB\\TestC\\A*data1->items"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items" type="array" children="1" numchildren="4" page="0" pagesize="32"><property name="0" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[0]" type="int"><![CDATA[1]]></property><property name="1" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[1]" type="int"><![CDATA[2]]></property><property name="2" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[2]" type="int"><![CDATA[3]]></property><property name="3" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[3]" type="int"><![CDATA[4]]></property></property></response>
