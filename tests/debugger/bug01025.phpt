--TEST--
Test for bug #1025: Xdebug does not reject wrong breakpoint types (-t)
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

$filename = dirname(__FILE__) . '/bug01025.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t bakerstreet',
	'breakpoint_set -t line -n 3',
	'breakpoint_set -t conditional -n 4',
	'breakpoint_set -t call -m strlen',
	'breakpoint_set -t return -m strlen',
	'breakpoint_set -t exception -x Exception',
	'breakpoint_set -t watch',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01025.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01025.inc" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t bakerstreet
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" status="break" reason="ok"><error code="3"><message><![CDATA[invalid or missing options]]></message></error></response>

-> breakpoint_set -i 3 -t line -n 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="{{PID}}0001"></response>

-> breakpoint_set -i 4 -t conditional -n 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="4" id="{{PID}}0002"></response>

-> breakpoint_set -i 5 -t call -m strlen
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="5" id="{{PID}}0003"></response>

-> breakpoint_set -i 6 -t return -m strlen
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="6" id="{{PID}}0004"></response>

-> breakpoint_set -i 7 -t exception -x Exception
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="7" id="{{PID}}0005"></response>

-> breakpoint_set -i 8 -t watch
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="8" status="break" reason="ok"><error code="201"><message><![CDATA[breakpoint type is not supported]]></message></error></response>
