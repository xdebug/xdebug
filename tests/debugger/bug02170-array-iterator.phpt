--TEST--
Test for bug #2170: Support ArrayIterator
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug02170-array-iterator.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 3",
	'run',
	'property_get -n $array',
	'property_get -n $array->storage',
	'property_get -n $array->storage[2]',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02170-array-iterator.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug02170-array-iterator.inc -n 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="{{PID}}0001"></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug02170-array-iterator.inc" lineno="3"></xdebug:message></response>

-> property_get -i 3 -n $array
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="3"><property name="$array" fullname="$array" type="object" classname="ArrayIterator" children="1" numchildren="1" page="0" pagesize="32"><property name="storage" fullname="$array-&gt;storage" facet="private" type="array" children="1" numchildren="4"></property></property></response>

-> property_get -i 4 -n $array->storage
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$array-&gt;storage" fullname="$array-&gt;storage" type="array" children="1" numchildren="4" page="0" pagesize="32"><property name="0" fullname="$array-&gt;storage[0]" type="int"><![CDATA[2]]></property><property name="1" fullname="$array-&gt;storage[1]" type="int"><![CDATA[13]]></property><property name="2" fullname="$array-&gt;storage[2]" type="int"><![CDATA[15]]></property><property name="3" fullname="$array-&gt;storage[3]" type="int"><![CDATA[17]]></property></property></response>

-> property_get -i 5 -n $array->storage[2]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$array-&gt;storage[2]" fullname="$array-&gt;storage[2]" type="int"><![CDATA[15]]></property></response>

-> detach -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="6" status="stopping" reason="ok"></response>
