--TEST--
Test for bug #987: Hidden property names not shown while debugging
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug00987-005.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 16',
	'run',
	'property_get -n $o',
	'property_get -n $o->-4',
	'property_get -n $o->{"-4"}',
	'property_get -n $o->3',
	'property_get -n $o->-2[2]',
	'property_get -n $o->-2[8]',
	'property_get -n $o->-2[8]["c"]',
	'property_get -n $o->5',
	'property_get -n $o->5->bar',
	'property_get -n $o->5->baz[2]',
	'property_get -n $o->5->b',
	'property_get -n $o->5->b::foo',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00987-005.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug00987-005.inc" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 16
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug00987-005.inc" lineno="16"></xdebug:message></response>

-> property_get -i 4 -n $o
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$o" fullname="$o" type="object" classname="stdClass" children="1" numchildren="6" page="0" pagesize="32"><property name="key" fullname="$o-&gt;key" facet="public" type="string" size="5" encoding="base64"><![CDATA[dmFsdWU=]]></property><property name="1" fullname="$o-&gt;1" facet="public" type="int"><![CDATA[0]]></property><property name="-4" fullname="$o-&gt;{&quot;-4&quot;}" facet="public" type="string" size="3" encoding="base64"><![CDATA[Zm9v]]></property><property name="3" fullname="$o-&gt;3" facet="public" type="bool"><![CDATA[0]]></property><property name="-2" fullname="$o-&gt;{&quot;-2&quot;}" facet="public" type="array" children="1" numchildren="4"></property><property name="5" fullname="$o-&gt;5" facet="public" type="object" classname="stdClass" children="1" numchildren="4"></property></property></response>

-> property_get -i 5 -n $o->-4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$o-&gt;-4" fullname="$o-&gt;-4" type="string" size="3" encoding="base64"><![CDATA[Zm9v]]></property></response>

-> property_get -i 6 -n $o->{"-4"}
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$o-&gt;{&quot;-4&quot;}" fullname="$o-&gt;{&quot;-4&quot;}" type="string" size="3" encoding="base64"><![CDATA[Zm9v]]></property></response>

-> property_get -i 7 -n $o->3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$o-&gt;3" fullname="$o-&gt;3" type="bool"><![CDATA[0]]></property></response>

-> property_get -i 8 -n $o->-2[2]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$o-&gt;-2[2]" fullname="$o-&gt;-2[2]" type="int"><![CDATA[7]]></property></response>

-> property_get -i 9 -n $o->-2[8]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="$o-&gt;-2[8]" fullname="$o-&gt;-2[8]" type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="0" fullname="$o-&gt;-2[8][0]" type="string" size="1" encoding="base64"><![CDATA[YQ==]]></property><property name="1" fullname="$o-&gt;-2[8][1]" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property><property name="2" fullname="$o-&gt;-2[8][2]" type="string" size="1" encoding="base64"><![CDATA[Yw==]]></property></property></response>

-> property_get -i 10 -n $o->-2[8]["c"]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="10" status="break" reason="ok"><error code="300"><message><![CDATA[can not get property]]></message></error></response>

-> property_get -i 11 -n $o->5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="11"><property name="$o-&gt;5" fullname="$o-&gt;5" type="object" classname="stdClass" children="1" numchildren="4" page="0" pagesize="32"><property name="foo" fullname="$o-&gt;5-&gt;foo" facet="public" type="int"><![CDATA[1]]></property><property name="bar" fullname="$o-&gt;5-&gt;bar" facet="public" type="int"><![CDATA[2]]></property><property name="baz" fullname="$o-&gt;5-&gt;baz" facet="public" type="array" children="1" numchildren="3"></property><property name="b" fullname="$o-&gt;5-&gt;b" facet="public" type="object" classname="b" children="1" numchildren="1"></property></property></response>

-> property_get -i 12 -n $o->5->bar
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="12"><property name="$o-&gt;5-&gt;bar" fullname="$o-&gt;5-&gt;bar" type="int"><![CDATA[2]]></property></response>

-> property_get -i 13 -n $o->5->baz[2]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="13"><property name="$o-&gt;5-&gt;baz[2]" fullname="$o-&gt;5-&gt;baz[2]" type="string" size="3" encoding="base64"><![CDATA[Zm9v]]></property></response>

-> property_get -i 14 -n $o->5->b
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="14"><property name="$o-&gt;5-&gt;b" fullname="$o-&gt;5-&gt;b" type="object" classname="b" children="1" numchildren="1" page="0" pagesize="32"><property name="foo" fullname="$o-&gt;5-&gt;b::foo" facet="static public" type="int"><![CDATA[73]]></property></property></response>

-> property_get -i 15 -n $o->5->b::foo
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="15"><property name="$o-&gt;5-&gt;b::foo" fullname="$o-&gt;5-&gt;b::foo" type="int"><![CDATA[73]]></property></response>

-> detach -i 16
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="16" status="stopping" reason="ok"></response>
