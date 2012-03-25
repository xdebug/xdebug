--TEST--
Test for bug #797: XDebug terminates (or Eclipse loses it?) when viewing variables
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug00797.inc');

$commands = array(
	'breakpoint_set -t line -n 6 -f file:///tmp/xdebug-dbgp-test.php',
	'run',
	'stack_get',
	'stack_get -d 0',
	'context_get -d 1',
	'property_get -d 1 -n $this',
	'property_get -d 1 -n $this::property',
	'context_get -d 0',
	'property_get -d 0 -n ::',
	'property_get -d 0 -n ::property',
);

dbgpRun( $data, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///tmp/xdebug-dbgp-test.php" language="PHP" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2012 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -n 6 -f file:///tmp/xdebug-dbgp-test.php
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id=""></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="6"></xdebug:message></response>

-> stack_get -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="3"><stack where="Base::basetest" level="0" type="file" filename="file:///tmp/xdebug-dbgp-test.php" lineno="6"></stack><stack where="Test-&gt;test" level="1" type="file" filename="file:///tmp/xdebug-dbgp-test.php" lineno="12"></stack><stack where="{main}" level="2" type="file" filename="file:///tmp/xdebug-dbgp-test.php" lineno="16"></stack></response>

-> stack_get -i 4 -d 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="4"><stack where="Base::basetest" level="0" type="file" filename="file:///tmp/xdebug-dbgp-test.php" lineno="6"></stack></response>

-> context_get -i 5 -d 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="5" context="0"><property name="$this" fullname="$this" address="" type="object" classname="Test" children="1" numchildren="1" page="0" pagesize="32"><property name="property" fullname="$this::property" facet="static public" address="" type="bool"><![CDATA[1]]></property></property></response>

-> property_get -i 6 -d 1 -n $this
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$this" fullname="$this" address="" type="object" classname="Test" children="1" numchildren="1" page="0" pagesize="32"><property name="property" fullname="$this::property" facet="static public" address="" type="bool"><![CDATA[1]]></property></property></response>

-> property_get -i 7 -d 1 -n $this::property
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$this::property" fullname="$this::property" address="" type="bool"><![CDATA[1]]></property></response>

-> context_get -i 8 -d 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="8" context="0"><property name="::" fullname="::" type="object" classname="Base" children="1" numchildren="1"><property name="::property" fullname="::property" address="" type="bool" facet="static public"><![CDATA[1]]></property></property></response>

-> property_get -i 9 -d 0 -n ::
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9" status="break" reason="ok"><error code="300"><message><![CDATA[can not get property]]></message></error></response>

-> property_get -i 10 -d 0 -n ::property
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="10"><property name="::property" fullname="::property" address="" type="bool"><![CDATA[1]]></property></response>
