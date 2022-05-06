--TEST--
DBGP: return breakpoints with userland return value
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/dbgp-breakpoint-return-function-003.inc';

$commands = array(
	'feature_set -n breakpoint_details -v 1',
	'feature_set -n breakpoint_include_return_value -v 1',
	'breakpoint_set -t return -m breakOnMe',
	'breakpoint_set -t return -m __construct -a Foo',
	'breakpoint_set -t return -m product -a Foo',
	'run',
	'run',
	'run',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://dbgp-breakpoint-return-function-003.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n breakpoint_details -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="breakpoint_details" success="1"></response>

-> feature_set -i 2 -n breakpoint_include_return_value -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="breakpoint_include_return_value" success="1"></response>

-> breakpoint_set -i 3 -t return -m breakOnMe
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="{{PID}}0001"></response>

-> breakpoint_set -i 4 -t return -m __construct -a Foo
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="4" id="{{PID}}0002"></response>

-> breakpoint_set -i 5 -t return -m product -a Foo
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="5" id="{{PID}}0003"></response>

-> run -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="6" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-return-function-003.inc" lineno="22"></xdebug:message><breakpoint type="return" function="__construct" class="Foo" state="enabled" hit_count="1" hit_value="0" id="{{PID}}0002"></breakpoint></response>

-> run -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="7" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-return-function-003.inc" lineno="25"></xdebug:message><xdebug:return_value><property type="object" classname="Foo" children="1" numchildren="2" page="0" pagesize="32"><property name="x" facet="public" type="int"><![CDATA[42]]></property><property name="y" facet="public" type="float"><![CDATA[2.7]]></property></property></xdebug:return_value><breakpoint type="return" function="breakOnMe" state="enabled" hit_count="1" hit_value="0" id="{{PID}}0001"></breakpoint></response>

-> run -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="8" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-return-function-003.inc" lineno="27"></xdebug:message><xdebug:return_value><property type="float"><![CDATA[113.4]]></property></xdebug:return_value><breakpoint type="return" function="product" class="Foo" state="enabled" hit_count="1" hit_value="0" id="{{PID}}0003"></breakpoint></response>

-> detach -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="9" status="stopping" reason="ok"></response>
