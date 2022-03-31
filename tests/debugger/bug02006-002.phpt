--TEST--
Test for bug #2006: Removing second call breakpoint with same function name crashes
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; NTS; !win');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02006.inc';

$commands = array(
	'breakpoint_set -t call -m a',
	'breakpoint_set -t call -m a',
	'breakpoint_list',
	'breakpoint_remove -d {{PID}}0001',
	'breakpoint_remove -d {{PID}}0002',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02006.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t call -m a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="{{PID}}0001"></response>

-> breakpoint_set -i 2 -t call -m a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" status="starting" reason="ok"><error code="200"><message><![CDATA[breakpoint could not be set]]></message></error></response>

-> breakpoint_list -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_list" transaction_id="3"><breakpoint type="call" function="a" state="enabled" hit_count="0" hit_value="0" id="{{PID}}0001"></breakpoint></response>

-> breakpoint_remove -i 4 -d %d0001
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_remove" transaction_id="4"><breakpoint type="call" function="a" state="enabled" hit_count="0" hit_value="0" id="{{PID}}0001"></breakpoint></response>

-> breakpoint_remove -i 5 -d %d0002
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_remove" transaction_id="5" status="starting" reason="ok"><error code="205"><message><![CDATA[no such breakpoint]]></message></error></response>

-> detach -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="6" status="stopping" reason="ok"></response>
