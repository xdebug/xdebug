--TEST--
Test for bug #1516: Can't fetch variables or object properties which have \0 characters in them
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01516.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 8',
	'run',
	'context_get',
	'property_get -d 0 -c 0 -n "with_\\0_null_char"',
	'property_get -d 0 -c 0 -n "$obj->with_\\0_null_char"',
	'feature_set -n extended_properties -v 1',
	'context_get',
	'property_get -d 0 -c 0 -n "with_\\0_null_char"',
	'property_get -d 0 -c 0 -n "$obj->with_\\0_null_char"',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01516.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01516.inc" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug01516.inc" lineno="8"></xdebug:message></response>

-> context_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property name="$name" fullname="$name" type="string" size="16" encoding="base64"><![CDATA[d2l0aF8AX251bGxfY2hhcg==]]></property><property name="$obj" fullname="$obj" type="object" classname="stdClass" children="1" numchildren="1" page="0" pagesize="32"><property name="with_&#0;_null_char" fullname="$obj-&gt;with_&#0;_null_char" facet="public" type="int"><![CDATA[42]]></property></property><property name="$with_&#0;_null_char" fullname="$with_&#0;_null_char" type="int"><![CDATA[42]]></property></response>

-> property_get -i 5 -d 0 -c 0 -n "with_\0_null_char"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$with_&#0;_null_char" fullname="$with_&#0;_null_char" type="int"><![CDATA[42]]></property></response>

-> property_get -i 6 -d 0 -c 0 -n "$obj->with_\0_null_char"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$obj-&gt;with_&#0;_null_char" fullname="$obj-&gt;with_&#0;_null_char" type="int"><![CDATA[42]]></property></response>

-> feature_set -i 7 -n extended_properties -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="7" feature="extended_properties" success="1"></response>

-> context_get -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="8" context="0"><property type="string" size="16"><name encoding="base64"><![CDATA[JG5hbWU=]]></name><fullname encoding="base64"><![CDATA[JG5hbWU=]]></fullname><value encoding="base64"><![CDATA[d2l0aF8AX251bGxfY2hhcg==]]></value></property><property name="$obj" fullname="$obj" type="object" classname="stdClass" children="1" numchildren="1" page="0" pagesize="32"><property facet="public" type="int"><name encoding="base64"><![CDATA[d2l0aF8AX251bGxfY2hhcg==]]></name><fullname encoding="base64"><![CDATA[JG9iai0+d2l0aF8AX251bGxfY2hhcg==]]></fullname><value encoding="base64"><![CDATA[NDI=]]></value></property></property><property type="int"><name encoding="base64"><![CDATA[JHdpdGhfAF9udWxsX2NoYXI=]]></name><fullname encoding="base64"><![CDATA[JHdpdGhfAF9udWxsX2NoYXI=]]></fullname><value encoding="base64"><![CDATA[NDI=]]></value></property></response>

-> property_get -i 9 -d 0 -c 0 -n "with_\0_null_char"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property type="int"><name encoding="base64"><![CDATA[JHdpdGhfAF9udWxsX2NoYXI=]]></name><fullname encoding="base64"><![CDATA[JHdpdGhfAF9udWxsX2NoYXI=]]></fullname><value encoding="base64"><![CDATA[NDI=]]></value></property></response>

-> property_get -i 10 -d 0 -c 0 -n "$obj->with_\0_null_char"
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="10"><property type="int"><name encoding="base64"><![CDATA[JG9iai0+d2l0aF8AX251bGxfY2hhcg==]]></name><fullname encoding="base64"><![CDATA[JG9iai0+d2l0aF8AX251bGxfY2hhcg==]]></fullname><value encoding="base64"><![CDATA[NDI=]]></value></property></response>
