--TEST--
Test for bug #2180: Crash with extended SplFixedArray
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02180.inc';

$commands = array(
	'step_into',
	'step_into',
	'step_into',
	'context_get -c 0',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02180.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug02180.inc" lineno="5"></xdebug:message></response>

-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug02180.inc" lineno="6"></xdebug:message></response>

-> step_into -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug02180.inc" lineno="7"></xdebug:message></response>

-> context_get -i 4 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property name="$foo" fullname="$foo" type="object" classname="Foo" children="1" numchildren="7" page="0" pagesize="32"><property name="0" fullname="$foo-&gt;0" facet="public" type="null"></property><property name="1" fullname="$foo-&gt;1" facet="public" type="null"></property><property name="2" fullname="$foo-&gt;2" facet="public" type="null"></property><property name="3" fullname="$foo-&gt;3" facet="public" type="null"></property><property name="4" fullname="$foo-&gt;4" facet="public" type="null"></property><property name="5" fullname="$foo-&gt;5" facet="public" type="null"></property><property name="6" fullname="$foo-&gt;6" facet="public" type="null"></property></property></response>

-> detach -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="5" status="stopping" reason="ok"></response>
