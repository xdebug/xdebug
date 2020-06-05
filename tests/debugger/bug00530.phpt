--TEST--
Test for bug #530: Xdebug returns properties out of page if there are less than max_children properties
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug00530.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 12",
	'run',
	'property_get -n a',
	'property_get -n this',
	'property_get -n this -p 0',
	'property_get -n this -p 1',
	'detach'
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00530.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug00530.inc -n 12
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id=""></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug00530.inc" lineno="12"></xdebug:message></response>

-> property_get -i 3 -n a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="3"><property name="$a" fullname="$a" type="string" size="4" encoding="base64"><![CDATA[Z29nbw==]]></property></response>

-> property_get -i 4 -n this
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$this" fullname="$this" type="object" classname="MyClass" children="1" numchildren="2" page="0" pagesize="32"><property name="a" fullname="$this-&gt;a" facet="private" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property><property name="b" fullname="$this-&gt;b" facet="private" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property></property></response>

-> property_get -i 5 -n this -p 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$this" fullname="$this" type="object" classname="MyClass" children="1" numchildren="2" page="0" pagesize="32"><property name="a" fullname="$this-&gt;a" facet="private" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property><property name="b" fullname="$this-&gt;b" facet="private" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property></property></response>

-> property_get -i 6 -n this -p 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$this" fullname="$this" type="object" classname="MyClass" children="1" numchildren="2" page="1" pagesize="32"></property></response>

-> detach -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="7" status="stopping" reason="ok"></response>
