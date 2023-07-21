--TEST--
Test for bug #566: Xdebug crashes when using conditional breakpoints (2)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--INI--
xdebug.collect_return=1
xdebug.collect_assignments=0
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug00566-002.inc' );

$commands = array(
	"breakpoint_set -n 7 -t conditional -f file://{$filename} -- JG1vZHVsZSA9PSAidmlld3Mi",
	'run',
	'context_get',
	'detach'
);

dbgpRunFile( $filename, $commands, [ 'track_errors' => 'Off' ] );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00566-002.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -n 7 -t conditional -f file://bug00566-002.inc -- JG1vZHVsZSA9PSAidmlld3Mi
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="{{PID}}0001"></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug00566-002.inc" lineno="7"></xdebug:message></response>

-> context_get -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="3" context="0"><property name="$module" fullname="$module" type="string" size="5" encoding="base64"><![CDATA[dmlld3M=]]></property></response>

-> detach -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="4" status="stopping" reason="ok"></response>
