--TEST--
Test for bug #678: Xdebug segfault when IDE send "eval NonExistsClass"
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug00678-001.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 4",
	'run',
	'stack_get',
	'eval -- bmV3IE5vbkV4aXN0c0NsYXNz',
	'stack_get',
	'step_into',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00678-001.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug00678-001.inc -n 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id=""></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug00678-001.inc" lineno="4"></xdebug:message></response>

-> stack_get -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="3"><stack where="test" level="0" type="file" filename="file://bug00678-001.inc" lineno="4"></stack><stack where="{main}" level="1" type="file" filename="file://bug00678-001.inc" lineno="6"></stack></response>

-> eval -i 4 -- bmV3IE5vbkV4aXN0c0NsYXNz
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="4" status="break" reason="ok"><error code="206"><message><![CDATA[error evaluating code]]></message></error></response>

-> stack_get -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="5"><stack where="test" level="0" type="file" filename="file://bug00678-001.inc" lineno="4"></stack><stack where="{main}" level="1" type="file" filename="file://bug00678-001.inc" lineno="6"></stack></response>

-> step_into -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="6" status="break" reason="ok"><xdebug:message filename="file://bug00678-001.inc" lineno="7"></xdebug:message></response>
