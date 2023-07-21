--TEST--
Test for bug #839: xdebug shows wrong data in debug (when using static class vars?)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug00839.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 42",
	'run',
	'property_get -n $a',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00839.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug00839.inc -n 42
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="{{PID}}0001"></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug00839.inc" lineno="42"></xdebug:message></response>

-> property_get -i 3 -n $a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="3"><property name="$a" fullname="$a" type="object" classname="A" children="1" numchildren="3" page="0" pagesize="32"><property name="_dumpTypeValues" fullname="$a::_dumpTypeValues" facet="static private" type="array" children="1" numchildren="3"></property><property name="prop1" fullname="$a-&gt;prop1" facet="protected" type="null"></property><property name="prop2" fullname="$a-&gt;prop2" facet="protected" type="null"></property></property></response>
