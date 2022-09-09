--TEST--
Test for bug #2055: Debugger creates XML with double facet attribute (< PHP 8.2)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP < 8.2; dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02055.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 14',
	'run',
	'context_get',
	'property_get -n $t',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02055.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug02055.inc" lineno="%r(2|18)%r"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 14
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug02055.inc" lineno="14"></xdebug:message></response>

-> context_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property name="$a" fullname="$a" type="int"><![CDATA[5]]></property><property name="$b" fullname="$b" type="int"><![CDATA[8]]></property><property name="$c" fullname="$c" type="object" facet="closure" classname="Closure" children="1" numchildren="2" page="0" pagesize="32"><property facet="virtual readonly" name="{closure}" type="array" children="1" page="0" pagesize="2" numchildren="2"><property facet="readonly" name="scope" type="string"><![CDATA[$this]]></property><property facet="readonly" name="function" type="string"><![CDATA[{closure}]]></property></property><property name="parameter" fullname="$c-&gt;parameter" facet="public" type="array" children="1" numchildren="2"></property></property><property name="::" fullname="::" type="object" classname="Test2055" children="1" numchildren="1"><property name="::c" fullname="::c" type="object" facet="closure static protected" classname="Closure" children="1" numchildren="2" page="0" pagesize="32"><property facet="virtual readonly" name="{closure}" type="array" children="1" page="0" pagesize="2" numchildren="2"><property facet="readonly" name="scope" type="string"><![CDATA[$this]]></property><property facet="readonly" name="function" type="string"><![CDATA[{closure}]]></property></property><property name="parameter" fullname="::c-&gt;parameter" facet="public" type="array" children="1" numchildren="2"></property></property></property></response>

-> property_get -i 5 -n $t
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5" status="break" reason="ok"><error code="300"><message><![CDATA[can not get property]]></message></error></response>

-> detach -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="6" status="stopping" reason="ok"></response>
