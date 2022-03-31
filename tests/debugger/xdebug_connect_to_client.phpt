--TEST--
xdebug_connect_to_client()
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$filename = dirname(__FILE__) . '/xdebug_connect_to_client.inc';

$commands = array(
	'stack_get',
	'property_get -n $i',
	'breakpoint_set -t line -n 8 -- ' . base64_encode('$i == 4'),
	'run',
	'stack_get',
	'property_get -n $i',
	'detach',
);

$settings = [
	'xdebug.mode' => 'debug',
	'xdebug.start_with_request' => 'trigger',
];

dbgpRunFile( $filename, $commands, $settings );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://xdebug_connect_to_client.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> stack_get -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="1"><stack where="worker" level="0" type="file" filename="file://xdebug_connect_to_client.inc" lineno="8"></stack><stack where="{main}" level="1" type="file" filename="file://xdebug_connect_to_client.inc" lineno="13"></stack></response>

-> property_get -i 2 -n $i
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="2"><property name="$i" fullname="$i" type="int"><![CDATA[1]]></property></response>

-> breakpoint_set -i 3 -t line -n 8 -- JGkgPT0gNA==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="{{PID}}0001"></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://xdebug_connect_to_client.inc" lineno="8"></xdebug:message></response>

-> stack_get -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="5"><stack where="worker" level="0" type="file" filename="file://xdebug_connect_to_client.inc" lineno="8"></stack><stack where="{main}" level="1" type="file" filename="file://xdebug_connect_to_client.inc" lineno="13"></stack></response>

-> property_get -i 6 -n $i
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$i" fullname="$i" type="int"><![CDATA[4]]></property></response>

-> detach -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="7" status="stopping" reason="ok"></response>
