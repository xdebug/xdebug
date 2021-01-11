--TEST--
Test for bug #1931: No local variables with trigger and xdebug_break() [3]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$filename = dirname(__FILE__) . '/bug01931-003.inc';

$commands = array(
	'run',
	'stack_get',
	'context_get -d 0',
	'context_get -d 1',
	'context_get -d 2',
);

$settings = [
	'xdebug.mode' => 'debug',
	'xdebug.start_with_request' => 'trigger',
];

dbgpRunFile( $filename, $commands, $settings );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01931-003.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> run -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01931-003.inc" lineno="9"></xdebug:message></response>

-> stack_get -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="2"><stack where="test" level="0" type="file" filename="file://bug01931-003.inc" lineno="9"></stack><stack where="array_map" level="1" type="file" filename="file://bug01931-003.inc" lineno="14"></stack><stack where="{main}" level="2" type="file" filename="file://bug01931-003.inc" lineno="14"></stack></response>

-> context_get -i 3 -d 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="3" context="0"><property name="$import" fullname="$import" type="string" size="3" encoding="base64"><![CDATA[b25l]]></property><property name="$local" fullname="$local" type="int"><![CDATA[1]]></property><property name="$local_three" fullname="$local_three" type="uninitialized"></property><property name="$local_two" fullname="$local_two" type="string" size="9" encoding="base64"><![CDATA[dGVzdDogb25l]]></property></response>

-> context_get -i 4 -d 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"></response>

-> context_get -i 5 -d 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="5" context="0"><property name="$array" fullname="$array" type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="0" fullname="$array[0]" type="string" size="3" encoding="base64"><![CDATA[b25l]]></property><property name="1" fullname="$array[1]" type="string" size="3" encoding="base64"><![CDATA[dHdv]]></property><property name="2" fullname="$array[2]" type="string" size="5" encoding="base64"><![CDATA[dGhyZWU=]]></property></property><property name="$main_local" fullname="$main_local" type="int"><![CDATA[42]]></property></response>
