--TEST--
DBGP: context_get
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/dbgp-context-get.inc';

$commands = array(
	'step_into',
	'step_into',
	'step_into',
	'context_get -c 0',
	'context_get -c 2',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://dbgp-context-get.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://dbgp-context-get.inc" lineno="2"></xdebug:message></response>

-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://dbgp-context-get.inc" lineno="2"></xdebug:message></response>

-> step_into -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://dbgp-context-get.inc" lineno="3"></xdebug:message></response>

-> context_get -i 4 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property name="$NO" fullname="$NO" type="int"><![CDATA[42]]></property></response>

-> context_get -i 5 -c 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="5" context="2"><property name="YES" fullname="YES" type="float" facet="constant"><![CDATA[3.1415926535898]]></property></response>
