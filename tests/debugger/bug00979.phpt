--TEST--
Test for bug #979: property_value -m 0 should mean all bytes, not 0 bytes
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug00979.inc';

$commands = array(
	'step_into',
	'feature_set -n max_data -v 32',
	'breakpoint_set -t line -n 3',
	'run',
	'property_get -n $a',
	'property_value -n $a',
	'property_value -n $a -m 0',
	'property_value -n $a -m "-1"',
	'property_value -n $a -m 20',
	'property_value -n $a',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00979.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug00979.inc" lineno="2"></xdebug:message></response>

-> feature_set -i 2 -n max_data -v 32
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="max_data" success="1"></response>

-> breakpoint_set -i 3 -t line -n 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="{{PID}}0001"></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://bug00979.inc" lineno="3"></xdebug:message></response>

-> property_get -i 5 -n $a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$a" fullname="$a" type="string" size="1280" encoding="base64"><![CDATA[YWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWI=]]></property></response>

-> property_value -i 6 -n $a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_value" transaction_id="6" type="string" size="1280" encoding="base64"><![CDATA[YWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWI=]]></response>

-> property_value -i 7 -n $a -m 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_value" transaction_id="7" type="string" size="1280" encoding="base64"><![CDATA[YWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWJjZGVmZ2hpamFiY2RlZmdoaWo=]]></response>

-> property_value -i 8 -n $a -m "-1"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_value" transaction_id="8" status="break" reason="ok"><error code="3"><message><![CDATA[invalid or missing options]]></message></error></response>

-> property_value -i 9 -n $a -m 20
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_value" transaction_id="9" type="string" size="1280" encoding="base64"><![CDATA[YWJjZGVmZ2hpamFiY2RlZmdoaWo=]]></response>

-> property_value -i 10 -n $a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_value" transaction_id="10" type="string" size="1280" encoding="base64"><![CDATA[YWJjZGVmZ2hpamFiY2RlZmdoaWphYmNkZWZnaGlqYWI=]]></response>

-> detach -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="11" status="stopping" reason="ok"></response>
