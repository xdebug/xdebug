--TEST--
Test for bug #2126: Problems with showing global variables
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02126.inc';

$commands = array(
	'step_into',
	'feature_set -n max_depth -v 0',
	'context_get -c 1',
	'breakpoint_set -t line -n 8',
	'run',
	'context_get -c 1',
	'feature_set -n max_depth -v 1',
	'property_get -c 1 -n $globalVar',
	'property_get -c 1 -n $argv',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02126.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug02126.inc" lineno="2"></xdebug:message></response>

-> feature_set -i 2 -n max_depth -v 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="max_depth" success="1"></response>

-> context_get -i 3 -c 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="3" context="1"><property name="$_GET" fullname="$_GET" type="array" children="0" numchildren="0"></property><property name="$_POST" fullname="$_POST" type="array" children="0" numchildren="0"></property><property name="$_COOKIE" fullname="$_COOKIE" type="array" children="0" numchildren="0"></property><property name="$_FILES" fullname="$_FILES" type="array" children="0" numchildren="0"></property><property name="$argv" fullname="$argv" type="array" children="1" numchildren="1"></property><property name="$argc" fullname="$argc" type="int"><![CDATA[1]]></property><property name="$_ENV" fullname="$_ENV" type="array" children="%d" numchildren="%d"></property><property name="$_REQUEST" fullname="$_REQUEST" type="array" children="0" numchildren="0"></property><property name="$_SERVER" fullname="$_SERVER" type="array" children="1" numchildren="%d"></property><property name="$globalVar" fullname="$globalVar" type="uninitialized"></property></response>

-> breakpoint_set -i 4 -t line -n 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="4" id="{{PID}}0001"></response>

-> run -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file://bug02126.inc" lineno="8"></xdebug:message></response>

-> context_get -i 6 -c 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="6" context="1"><property name="$_GET" fullname="$_GET" type="array" children="0" numchildren="0"></property><property name="$_POST" fullname="$_POST" type="array" children="0" numchildren="0"></property><property name="$_COOKIE" fullname="$_COOKIE" type="array" children="0" numchildren="0"></property><property name="$_FILES" fullname="$_FILES" type="array" children="0" numchildren="0"></property><property name="$argv" fullname="$argv" type="array" children="1" numchildren="1"></property><property name="$argc" fullname="$argc" type="int"><![CDATA[1]]></property><property name="$_ENV" fullname="$_ENV" type="array" children="%d" numchildren="%d"></property><property name="$_REQUEST" fullname="$_REQUEST" type="array" children="0" numchildren="0"></property><property name="$_SERVER" fullname="$_SERVER" type="array" children="1" numchildren="%d"></property><property name="$globalVar" fullname="$globalVar" type="int"><![CDATA[42]]></property></response>

-> feature_set -i 7 -n max_depth -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="7" feature="max_depth" success="1"></response>

-> property_get -i 8 -c 1 -n $globalVar
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$globalVar" fullname="$globalVar" type="int"><![CDATA[42]]></property></response>

-> property_get -i 9 -c 1 -n $argv
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="$argv" fullname="$argv" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="0" fullname="$argv[0]" type="string" size="%d" encoding="base64"><![CDATA[%s]]></property></property></response>

-> detach -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="10" status="stopping" reason="ok"></response>
