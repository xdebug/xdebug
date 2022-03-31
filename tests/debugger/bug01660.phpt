--TEST--
Test for bug #1660: Return breakpoints for methods don't break immediately
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$dir = dirname(__FILE__);
putenv("XDEBUG_TEST_DIR=$dir");

$filename = realpath( dirname(__FILE__) . '/bug01388-14-index.inc' );

$commands = array(
	'feature_set -n notify_ok -v 1',
	'feature_set -n resolved_breakpoints -v 1',
	'breakpoint_set -t return -m SimpleClass::displayVar',
	'run',
	'context_get',
	'property_get -n ::hello'
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01388-14-index.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n notify_ok -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="notify_ok" success="1"></response>

-> feature_set -i 2 -n resolved_breakpoints -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="resolved_breakpoints" success="1"></response>

-> breakpoint_set -i 3 -t return -m SimpleClass::displayVar
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="{{PID}}0001" resolved="resolved"></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://bug01388-14-simpleclass.inc" lineno="8"></xdebug:message></response>

-> context_get -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="5" context="0"><property name="::" fullname="::" type="object" classname="SimpleClass" children="1" numchildren="1"><property name="::hello" fullname="::hello" type="string" size="5" facet="static public" encoding="base64"><![CDATA[SEVMTE8=]]></property></property></response>

-> property_get -i 6 -n ::hello
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="::hello" fullname="::hello" type="string" size="5" encoding="base64"><![CDATA[SEVMTE8=]]></property></response>
