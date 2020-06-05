--TEST--
Test for bug #840: Xdebug crashes when a class is returned by a __call method with a static var and more than 2 props
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug00840.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 31",
	'run',
	'context_get -d 0 -c 0',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00840.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug00840.inc -n 31
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id=""></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug00840.inc" lineno="31"></xdebug:message></response>

-> context_get -i 3 -d 0 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="3" context="0"><property name="$b" fullname="$b" type="object" classname="B" children="0" numchildren="0" page="0" pagesize="32"></property><property name="$x" fullname="$x" type="object" classname="A" children="1" numchildren="4" page="0" pagesize="32"><property name="_staticvar" fullname="$x::_staticvar" facet="static public" type="null"></property><property name="var_1" fullname="$x-&gt;var_1" facet="protected" type="null"></property><property name="var_2" fullname="$x-&gt;var_2" facet="protected" type="null"></property><property name="var_3" fullname="$x-&gt;var_3" facet="protected" type="null"></property></property></response>
