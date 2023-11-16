--TEST--
DBGP: virtual __EXCEPTION local property when feature virtual_exception_value is set
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/dbgp-virtual-exception-value-001.inc';

$commands = array(
	'feature_set -n virtual_exception_value -v 1',
	'breakpoint_set -t exception -x Exception',
	'run',
	'context_get',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://dbgp-virtual-exception-value-001.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n virtual_exception_value -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="virtual_exception_value" success="1"></response>

-> breakpoint_set -i 2 -t exception -x Exception
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://dbgp-virtual-exception-value-001.inc" lineno="2" exception="Exception"><![CDATA[TEST]]></xdebug:message></response>

-> context_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property name="$__EXCEPTION" fullname="$__EXCEPTION" type="object" classname="Exception" children="1" numchildren="7" page="0" pagesize="32" facet="readonly virtual"><property name="message" fullname="$__EXCEPTION-&gt;message" facet="protected" type="string" size="4" encoding="base64"><![CDATA[VEVTVA==]]></property><property name="string" fullname="$__EXCEPTION-&gt;string" facet="private" type="string" size="0" encoding="base64"><![CDATA[]]></property><property name="code" fullname="$__EXCEPTION-&gt;code" facet="protected" type="int"><![CDATA[0]]></property><property name="file" fullname="$__EXCEPTION-&gt;file" facet="protected" type="string" size="%d" encoding="base64"><![CDATA[%s]]></property><property name="line" fullname="$__EXCEPTION-&gt;line" facet="protected" type="int"><![CDATA[2]]></property><property name="trace" fullname="$__EXCEPTION-&gt;trace" facet="private" type="array" children="0" numchildren="0"></property><property name="previous" fullname="$__EXCEPTION-&gt;previous" facet="private" type="object" classname="Exception" children="1" numchildren="7"></property></property></response>

-> detach -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="5" status="stopping" reason="ok"></response>
