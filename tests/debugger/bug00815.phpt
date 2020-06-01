--TEST--
Test for bug #815: Xdebug crashes when 'exit' operator used in the script
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug00815.inc';

$commands = array(
	'feature_set -n show_hidden -v 1',
	'feature_set -n max_depth -v 1',
	'feature_set -n max_children -v 100',
	'status',
	'step_into',
	'eval -- aXNzZXQoJF9TRVJWRVJbJ1BIUF9JREVfQ09ORklHJ10p',
	'eval -- aXNzZXQoJF9TRVJWRVJbJ1NFUlZFUl9OQU1FJ10p',
	'eval -- KHN0cmluZykoJF9TRVJWRVJbJ1NFUlZFUl9OQU1FJ10p',
	'eval -- KHN0cmluZykoJF9TRVJWRVJbJ1NFUlZFUl9QT1JUJ10p',
	'eval -- KHN0cmluZykoJF9TRVJWRVJbJ1JFUVVFU1RfVVJJJ10p',
	'breakpoint_set -t line -n 2',
	'breakpoint_set -t line -n 2',
	'stack_get',
	'stack_get',
	'context_names',
	'run',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00815.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n show_hidden -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="show_hidden" success="1"></response>

-> feature_set -i 2 -n max_depth -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="max_depth" success="1"></response>

-> feature_set -i 3 -n max_children -v 100
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="3" feature="max_children" success="1"></response>

-> status -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="status" transaction_id="4" status="starting" reason="ok"></response>

-> step_into -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file://bug00815.inc" lineno="2"></xdebug:message></response>

-> eval -i 6 -- aXNzZXQoJF9TRVJWRVJbJ1BIUF9JREVfQ09ORklHJ10p
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="6"><property type="bool"><![CDATA[0]]></property></response>

-> eval -i 7 -- aXNzZXQoJF9TRVJWRVJbJ1NFUlZFUl9OQU1FJ10p
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="7"><property type="bool"><![CDATA[0]]></property></response>

-> eval -i 8 -- KHN0cmluZykoJF9TRVJWRVJbJ1NFUlZFUl9OQU1FJ10p
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="8"><property type="string" size="0" encoding="base64"><![CDATA[]]></property></response>

-> eval -i 9 -- KHN0cmluZykoJF9TRVJWRVJbJ1NFUlZFUl9QT1JUJ10p
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="9"><property type="string" size="0" encoding="base64"><![CDATA[]]></property></response>

-> eval -i 10 -- KHN0cmluZykoJF9TRVJWRVJbJ1JFUVVFU1RfVVJJJ10p
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="10"><property type="string" size="0" encoding="base64"><![CDATA[]]></property></response>

-> breakpoint_set -i 11 -t line -n 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="11" id=""></response>

-> breakpoint_set -i 12 -t line -n 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="12" id=""></response>

-> stack_get -i 13
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="13"><stack where="{main}" level="0" type="file" filename="file://bug00815.inc" lineno="2"></stack></response>

-> stack_get -i 14
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="14"><stack where="{main}" level="0" type="file" filename="file://bug00815.inc" lineno="2"></stack></response>

-> context_names -i 15
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_names" transaction_id="15"><context name="Locals" id=""></context><context name="Superglobals" id=""></context><context name="User defined constants" id=""></context></response>

-> run -i 16
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="16" status="stopping" reason="ok"></response>
