--TEST--
Test for bug #1931: No local variables with trigger and xdebug_break() [1]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$filename = dirname(__FILE__) . '/bug01931-001.inc';

$commands = array(
	'run',
	'stack_get',
	'context_get -d 0',
	'run',
	'stack_get',
	'context_get -d 0',
);

$settings = [
	'xdebug.mode' => 'debug',
	'xdebug.start_with_request' => 'trigger',
];

dbgpRunFile( $filename, $commands, $settings );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01931-001.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> run -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01931-001.inc" lineno="3"></xdebug:message></response>

-> stack_get -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="2"><stack where="{main}" level="0" type="file" filename="file://bug01931-001.inc" lineno="3"></stack></response>

-> context_get -i 3 -d 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="3" context="0"><property name="$local" fullname="$local" type="uninitialized"></property><property name="$local_two" fullname="$local_two" type="uninitialized"></property></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://bug01931-001.inc" lineno="9"></xdebug:message></response>

-> stack_get -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="5"><stack where="{main}" level="0" type="file" filename="file://bug01931-001.inc" lineno="9"></stack></response>

-> context_get -i 6 -d 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="6" context="0"><property name="$local" fullname="$local" type="int"><![CDATA[1]]></property><property name="$local_two" fullname="$local_two" type="string" size="3" encoding="base64"><![CDATA[dHdv]]></property></response>
