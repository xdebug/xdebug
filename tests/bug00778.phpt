--TEST--
Test for bug #778: Xdebug session in Eclipse crash whenever it run into simplexml_load_string call
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug00778.inc');

$commands = array(
	'step_into',
	'step_into',
	'step_into',
	'stack_get',
	'context_get',
	'detach',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///tmp/xdebug-dbgp-test.php" language="PHP" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-%d by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="4"></xdebug:message></response>

-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="5"></xdebug:message></response>

-> step_into -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="6"></xdebug:message></response>

-> stack_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="4"><stack where="{main}" level="0" type="file" filename="file:///tmp/xdebug-dbgp-test.php" lineno="6"></stack></response>

-> context_get -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="5" context="0"><property name="$temp" fullname="$temp" address="" type="object" classname="SimpleXMLElement" children="1" numchildren="1" page="0" pagesize="32"></property><property name="$xml" fullname="$xml" address="" type="string" size="54" encoding="base64"><![CDATA[PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iVVRGLTgiID8+Cjxyb290Pgo8L3Jvb3Q+]]></property></response>

-> detach -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="6" status="stopping" reason="ok"></response>
