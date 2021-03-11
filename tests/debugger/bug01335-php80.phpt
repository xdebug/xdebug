--TEST--
Test for bug #1335: Debugging with PhpStorm sometimes gives "can not get property" (< PHP 8.1)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 8.1; dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug01335.inc' );

$commands = array(
	'feature_set -n notify_ok -v 1',
	'feature_set -n resolved_breakpoints -v 1',
	"breakpoint_set -t line -f file://{$filename} -n 39",
	'run',
	'context_get',
	'property_get -d 0 -c 0 -n $b',
	'property_get -d 0 -c 0 -n $b->*TestA\\TestB\\TestC\\A*data1',
	'property_get -d 0 -c 0 -n $b->*TestA\\TestB\\TestC\\A*data1->items',
	'property_get -d 0 -c 0 -n "$b->*TestA\\\\TestB\\\\TestC\\\\A*data1"',
	'property_get -d 0 -c 0 -n "$b->*TestA\\\\TestB\\\\TestC\\\\A*data1->items"',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01335.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n notify_ok -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="notify_ok" success="1"></response>

-> feature_set -i 2 -n resolved_breakpoints -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="resolved_breakpoints" success="1"></response>

-> breakpoint_set -i 3 -t line -f file://bug01335.inc -n 39
<?xml version="1.0" encoding="iso-8859-1"?>
<notify xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" name="breakpoint_resolved"><breakpoint type="line" resolved="resolved" filename="file://bug01335.inc" lineno="39" state="enabled" hit_count="0" hit_value="0" id=""></breakpoint></notify>

<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="" resolved="resolved"></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://bug01335.inc" lineno="39"></xdebug:message></response>

-> context_get -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="5" context="0"><property name="$b" fullname="$b" type="object" classname="TestA\TestB\TestC\B" children="1" numchildren="2" page="0" pagesize="32"><property name="data2" fullname="$b-&gt;data2" facet="private" type="object" classname="TestA\TestB\TestC\C" children="1" numchildren="1"></property><property name="*TestA\TestB\TestC\A*data1" fullname="$b-&gt;*TestA\TestB\TestC\A*data1" facet="private" type="object" classname="TestA\TestB\TestC\C" children="1" numchildren="1"></property></property><property name="$x" fullname="$x" type="uninitialized"></property></response>

-> property_get -i 6 -d 0 -c 0 -n $b
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$b" fullname="$b" type="object" classname="TestA\TestB\TestC\B" children="1" numchildren="2" page="0" pagesize="32"><property name="data2" fullname="$b-&gt;data2" facet="private" type="object" classname="TestA\TestB\TestC\C" children="1" numchildren="1"></property><property name="*TestA\TestB\TestC\A*data1" fullname="$b-&gt;*TestA\TestB\TestC\A*data1" facet="private" type="object" classname="TestA\TestB\TestC\C" children="1" numchildren="1"></property></property></response>

-> property_get -i 7 -d 0 -c 0 -n $b->*TestA\TestB\TestC\A*data1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$b-&gt;*TestA\TestB\TestC\A*data1" fullname="$b-&gt;*TestA\TestB\TestC\A*data1" type="object" classname="TestA\TestB\TestC\C" children="1" numchildren="1" page="0" pagesize="32"><property name="items" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items" facet="private" type="array" children="1" numchildren="4"></property></property></response>

-> property_get -i 8 -d 0 -c 0 -n $b->*TestA\TestB\TestC\A*data1->items
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items" type="array" children="1" numchildren="4" page="0" pagesize="32"><property name="0" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[0]" type="int"><![CDATA[1]]></property><property name="1" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[1]" type="int"><![CDATA[2]]></property><property name="2" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[2]" type="int"><![CDATA[3]]></property><property name="3" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[3]" type="int"><![CDATA[4]]></property></property></response>

-> property_get -i 9 -d 0 -c 0 -n "$b->*TestA\\TestB\\TestC\\A*data1"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="$b-&gt;*TestA\TestB\TestC\A*data1" fullname="$b-&gt;*TestA\TestB\TestC\A*data1" type="object" classname="TestA\TestB\TestC\C" children="1" numchildren="1" page="0" pagesize="32"><property name="items" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items" facet="private" type="array" children="1" numchildren="4"></property></property></response>

-> property_get -i 10 -d 0 -c 0 -n "$b->*TestA\\TestB\\TestC\\A*data1->items"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="10"><property name="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items" type="array" children="1" numchildren="4" page="0" pagesize="32"><property name="0" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[0]" type="int"><![CDATA[1]]></property><property name="1" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[1]" type="int"><![CDATA[2]]></property><property name="2" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[2]" type="int"><![CDATA[3]]></property><property name="3" fullname="$b-&gt;*TestA\TestB\TestC\A*data1-&gt;items[3]" type="int"><![CDATA[4]]></property></property></response>
