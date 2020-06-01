--TEST--
Test for bug #1385: Can not fetch IS_INDIRECT private properties
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01385.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 22',
	'run',
	'property_get -d 0 -c 0 -n $v->a->*Foo*a->a',
	'property_get -d 0 -c 0 -n $v->a->a'
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01385.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01385.inc" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 22
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug01385.inc" lineno="22"></xdebug:message></response>

-> property_get -i 4 -d 0 -c 0 -n $v->a->*Foo*a->a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$v-&gt;a-&gt;*Foo*a-&gt;a" fullname="$v-&gt;a-&gt;*Foo*a-&gt;a" type="object" classname="Foo" children="1" numchildren="1" page="0" pagesize="32"><property name="a" fullname="$v-&gt;a-&gt;*Foo*a-&gt;a-&gt;a" facet="private" type="int"><![CDATA[2]]></property></property></response>

-> property_get -i 5 -d 0 -c 0 -n $v->a->a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$v-&gt;a-&gt;a" fullname="$v-&gt;a-&gt;a" type="int"><![CDATA[1]]></property></response>
