--TEST--
Test for bug #1105: Setting properties without specifying a type only works in topmost frame
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug01105-002.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 5",
	'run',
	'property_set -n $var -d 0 -- ' . base64_encode('"scope0-modified"'),
	'property_set -n $var -d 1 -- ' . base64_encode('"scope1-modified"'),
	'property_set -n $var -d 2 -- ' . base64_encode('"scope2-modified"'),
	'property_get -n $var -d 0',
	'property_get -n $var -d 1',
	'property_get -n $var -d 2',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01105-002.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug01105-002.inc -n 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="{{PID}}0001"></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug01105-002.inc" lineno="5"></xdebug:message></response>

-> property_set -i 3 -n $var -d 0 -- InNjb3BlMC1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="3" success="1"></response>

-> property_set -i 4 -n $var -d 1 -- InNjb3BlMS1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="4" success="1"></response>

-> property_set -i 5 -n $var -d 2 -- InNjb3BlMi1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="5" success="1"></response>

-> property_get -i 6 -n $var -d 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$var" fullname="$var" type="string" size="15" encoding="base64"><![CDATA[c2NvcGUwLW1vZGlmaWVk]]></property></response>

-> property_get -i 7 -n $var -d 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$var" fullname="$var" type="string" size="15" encoding="base64"><![CDATA[c2NvcGUxLW1vZGlmaWVk]]></property></response>

-> property_get -i 8 -n $var -d 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$var" fullname="$var" type="string" size="15" encoding="base64"><![CDATA[c2NvcGUyLW1vZGlmaWVk]]></property></response>

-> detach -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="9" status="stopping" reason="ok"></response>
