--TEST--
Test for bug #1388: Resolved function return breakpoint
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$filename = realpath( dirname(__FILE__) . '/bug01388-15.inc' );

$commands = array(
	'feature_set -n notify_ok -v 1',
	'feature_set -n resolved_breakpoints -v 1',
	'breakpoint_set -t return -m test1',
	'breakpoint_set -t return -m test_class::test2',
	'breakpoint_set -t return -m test_class::test3',
	'run',
	'stack_get',
	'context_get',
	'run',
	'stack_get',
	'context_get',
	'run',
	'stack_get',
	'context_get',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01388-15.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n notify_ok -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="notify_ok" success="1"></response>

-> feature_set -i 2 -n resolved_breakpoints -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="resolved_breakpoints" success="1"></response>

-> breakpoint_set -i 3 -t return -m test1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="" resolved="resolved"></response>

-> breakpoint_set -i 4 -t return -m test_class::test2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="4" id="" resolved="resolved"></response>

-> breakpoint_set -i 5 -t return -m test_class::test3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="5" id="" resolved="resolved"></response>

-> run -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="6" status="break" reason="ok"><xdebug:message filename="file://bug01388-15.inc" lineno="29"></xdebug:message></response>

-> stack_get -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="7"><stack where="test1" level="0" type="file" filename="file://bug01388-15.inc" lineno="29"></stack><stack where="{main}" level="1" type="file" filename="file://bug01388-15.inc" lineno="29"></stack></response>

-> context_get -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="8" context="0"></response>

-> run -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="9" status="break" reason="ok"><xdebug:message filename="file://bug01388-15.inc" lineno="30"></xdebug:message></response>

-> stack_get -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="10"><stack where="test_class::test2" level="0" type="file" filename="file://bug01388-15.inc" lineno="30"></stack><stack where="{main}" level="1" type="file" filename="file://bug01388-15.inc" lineno="30"></stack></response>

-> context_get -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="11" context="0"><property name="::" fullname="::" type="object" classname="test_class" children="0" numchildren="0"></property></response>

-> run -i 12
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="12" status="break" reason="ok"><xdebug:message filename="file://bug01388-15.inc" lineno="33"></xdebug:message></response>

-> stack_get -i 13
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="13"><stack where="test_class-&gt;test3" level="0" type="file" filename="file://bug01388-15.inc" lineno="33"></stack><stack where="{main}" level="1" type="file" filename="file://bug01388-15.inc" lineno="33"></stack></response>

-> context_get -i 14
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="14" context="0"></response>
