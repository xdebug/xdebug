--TEST--
Test for debugging with fibers (>= PHP 8.1)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1; dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/fiber-001.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 39',
	'breakpoint_set -t line -n 66',
	'breakpoint_set -t line -n 70',
	'run',
	'stack_depth',
	'stack_get',
	'property_get -n $loop->nextId',
	'run',
	'stack_depth',
	'stack_get',
	'property_get -n $id',
	'run',
	'stack_depth',
	'stack_get',
	'property_get -n $loop->nextId',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://fiber-001.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://fiber-001.inc" lineno="50"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 39
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> breakpoint_set -i 3 -t line -n 66
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="{{PID}}0002"></response>

-> breakpoint_set -i 4 -t line -n 70
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="4" id="{{PID}}0003"></response>

-> run -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file://fiber-001.inc" lineno="66"></xdebug:message></response>

-> stack_depth -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_depth" transaction_id="6" depth="2"></response>

-> stack_get -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="7"><stack where="{closure:%sfiber-001.inc:63-73}" level="0" type="file" filename="file://fiber-001.inc" lineno="66"></stack><stack where="{fiber:%s}" level="1" type="file" filename="file://fiber-001.inc" lineno="76"></stack></response>

-> property_get -i 8 -n $loop->nextId
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$loop-&gt;nextId" fullname="$loop-&gt;nextId" type="string" size="1" encoding="base64"><![CDATA[YQ==]]></property></response>

-> run -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="9" status="break" reason="ok"><xdebug:message filename="file://fiber-001.inc" lineno="39"></xdebug:message></response>

-> stack_depth -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_depth" transaction_id="10" depth="2"></response>

-> stack_get -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="11"><stack where="EventLoop-&gt;defer" level="0" type="file" filename="file://fiber-001.inc" lineno="39"></stack><stack where="{main}" level="1" type="file" filename="file://fiber-001.inc" lineno="79"></stack></response>

-> property_get -i 12 -n $id
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="12"><property name="$id" fullname="$id" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property></response>

-> run -i 13
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="13" status="break" reason="ok"><xdebug:message filename="file://fiber-001.inc" lineno="70"></xdebug:message></response>

-> stack_depth -i 14
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_depth" transaction_id="14" depth="2"></response>

-> stack_get -i 15
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="15"><stack where="{closure:%sfiber-001.inc:63-73}" level="0" type="file" filename="file://fiber-001.inc" lineno="70"></stack><stack where="{fiber:%s}" level="1" type="file" filename="file://fiber-001.inc" lineno="76"></stack></response>

-> property_get -i 16 -n $loop->nextId
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="16"><property name="$loop-&gt;nextId" fullname="$loop-&gt;nextId" type="string" size="1" encoding="base64"><![CDATA[Yw==]]></property></response>

-> detach -i 17
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="17" status="stopping" reason="ok"></response>
