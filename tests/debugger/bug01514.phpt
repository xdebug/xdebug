--TEST--
Test for bug #1514: Variable names with a NULL char are cut off at NULL char
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01514.inc';

$commands = array(
	'step_into',
	'step_into',
	'step_into',
	'context_get',
	'feature_set -n extended_properties -v 1',
	'context_get',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01514.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01514.inc" lineno="2"></xdebug:message></response>

-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug01514.inc" lineno="3"></xdebug:message></response>

-> step_into -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug01514.inc" lineno="5"></xdebug:message></response>

-> context_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property name="$name" fullname="$name" type="string" size="16" encoding="base64"><![CDATA[d2l0aF8AX251bGxfY2hhcg==]]></property><property name="$with_&#0;_null_char" fullname="$with_&#0;_null_char" type="int"><![CDATA[42]]></property></response>

-> feature_set -i 5 -n extended_properties -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="5" feature="extended_properties" success="1"></response>

-> context_get -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="6" context="0"><property type="string" size="16"><name encoding="base64"><![CDATA[JG5hbWU=]]></name><fullname encoding="base64"><![CDATA[JG5hbWU=]]></fullname><value encoding="base64"><![CDATA[d2l0aF8AX251bGxfY2hhcg==]]></value></property><property type="int"><name encoding="base64"><![CDATA[JHdpdGhfAF9udWxsX2NoYXI=]]></name><fullname encoding="base64"><![CDATA[JHdpdGhfAF9udWxsX2NoYXI=]]></fullname><value encoding="base64"><![CDATA[NDI=]]></value></property></response>
