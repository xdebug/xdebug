--TEST--
Test for bug #1512: Xdebug returns escaped property name and fullname
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug01512.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 17",
	'run',
	'property_get -d 0 -c 0 -n $this',
	'property_get -d 0 -c 0 -n "$this"',
	'property_get -d 0 -c 0 -n $this->*TestA\\TestB\\TestC\\A*items',
	'property_get -d 0 -c 0 -n "$this->*TestA\\\\TestB\\\\TestC\\\\A*items"',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01512.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug01512.inc -n 17
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id=""></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug01512.inc" lineno="17"></xdebug:message></response>

-> property_get -i 3 -d 0 -c 0 -n $this
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="3"><property name="$this" fullname="$this" type="object" classname="TestA\TestB\TestC\B" children="1" numchildren="1" page="0" pagesize="32"><property name="*TestA\TestB\TestC\A*items" fullname="$this-&gt;*TestA\TestB\TestC\A*items" facet="private" type="array" children="1" numchildren="1"></property></property></response>

-> property_get -i 4 -d 0 -c 0 -n "$this"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$this" fullname="$this" type="object" classname="TestA\TestB\TestC\B" children="1" numchildren="1" page="0" pagesize="32"><property name="*TestA\TestB\TestC\A*items" fullname="$this-&gt;*TestA\TestB\TestC\A*items" facet="private" type="array" children="1" numchildren="1"></property></property></response>

-> property_get -i 5 -d 0 -c 0 -n $this->*TestA\TestB\TestC\A*items
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$this-&gt;*TestA\TestB\TestC\A*items" fullname="$this-&gt;*TestA\TestB\TestC\A*items" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="0" fullname="$this-&gt;*TestA\TestB\TestC\A*items[0]" type="array" children="1" numchildren="2"></property></property></response>

-> property_get -i 6 -d 0 -c 0 -n "$this->*TestA\\TestB\\TestC\\A*items"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$this-&gt;*TestA\TestB\TestC\A*items" fullname="$this-&gt;*TestA\TestB\TestC\A*items" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="0" fullname="$this-&gt;*TestA\TestB\TestC\A*items[0]" type="array" children="1" numchildren="2"></property></property></response>
