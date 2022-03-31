--TEST--
Test for bug #1223: Xdebug crashes on PHP 7 when doing a DBGp eval command
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01223.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 8',
	'run',
	'eval -- JGE=',
	'step_over',
	'eval -- JGE=',
	'step_over',
	'eval -- JGE=',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01223.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01223.inc" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug01223.inc" lineno="8"></xdebug:message></response>

-> eval -i 4 -- JGE=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="4"><property type="array" children="0" numchildren="0" page="0" pagesize="32"></property></response>

-> step_over -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file://bug01223.inc" lineno="9"></xdebug:message></response>

-> eval -i 6 -- JGE=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="6"><property type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="x" type="string" size="1" encoding="base64"><![CDATA[eQ==]]></property></property></response>

-> step_over -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="7" status="break" reason="ok"><xdebug:message filename="file://bug01223.inc" lineno="11"></xdebug:message></response>

-> eval -i 8 -- JGE=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="8"><property type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="x" type="string" size="1" encoding="base64"><![CDATA[eQ==]]></property><property name="y" type="array" children="1" numchildren="1"></property></property></response>

-> detach -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="9" status="stopping" reason="ok"></response>
