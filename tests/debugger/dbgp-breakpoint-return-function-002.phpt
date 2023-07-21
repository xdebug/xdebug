--TEST--
DBGP: return breakpoints with return value
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/dbgp-breakpoint-call-function.inc';

$commands = array(
	'feature_set -n breakpoint_details -v 1',
	'feature_set -n breakpoint_include_return_value -v 1',
	'breakpoint_set -t return -m breakOnMe',
	'breakpoint_set -t return -m array_reverse',
	'run',
	'run',
	'run',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://dbgp-breakpoint-call-function.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n breakpoint_details -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="breakpoint_details" success="1"></response>

-> feature_set -i 2 -n breakpoint_include_return_value -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="breakpoint_include_return_value" success="1"></response>

-> breakpoint_set -i 3 -t return -m breakOnMe
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="{{PID}}0001"></response>

-> breakpoint_set -i 4 -t return -m array_reverse
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="4" id="{{PID}}0002"></response>

-> run -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-call-function.inc" lineno="7"></xdebug:message><breakpoint type="return" function="breakOnMe" state="enabled" hit_count="1" hit_value="0" id="{{PID}}0001"></breakpoint></response>

-> run -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="6" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-call-function.inc" lineno="10"></xdebug:message><xdebug:return_value><property type="array" children="1" numchildren="8" page="0" pagesize="32"><property name="0" type="int"><![CDATA[7]]></property><property name="1" type="int"><![CDATA[6]]></property><property name="2" type="int"><![CDATA[5]]></property><property name="3" type="int"><![CDATA[4]]></property><property name="4" type="int"><![CDATA[3]]></property><property name="5" type="int"><![CDATA[2]]></property><property name="6" type="int"><![CDATA[1]]></property><property name="7" type="int"><![CDATA[0]]></property></property></xdebug:return_value><breakpoint type="return" function="array_reverse" state="enabled" hit_count="1" hit_value="0" id="{{PID}}0002"></breakpoint></response>

-> run -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="7" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-call-function.inc" lineno="13"></xdebug:message><xdebug:return_value><property type="array" children="1" numchildren="9" page="0" pagesize="32"><property name="0" type="int"><![CDATA[42]]></property><property name="1" type="int"><![CDATA[0]]></property><property name="2" type="int"><![CDATA[1]]></property><property name="3" type="int"><![CDATA[2]]></property><property name="4" type="int"><![CDATA[3]]></property><property name="5" type="int"><![CDATA[4]]></property><property name="6" type="int"><![CDATA[5]]></property><property name="7" type="int"><![CDATA[6]]></property><property name="8" type="int"><![CDATA[7]]></property></property></xdebug:return_value><breakpoint type="return" function="array_reverse" state="enabled" hit_count="2" hit_value="0" id="{{PID}}0002"></breakpoint></response>

-> detach -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="8" status="stopping" reason="ok"></response>
