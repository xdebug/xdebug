--TEST--
Test for bug #2053: "Cannot get property" for array elements while evaluating
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02053.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 22',
	'run',
	'property_get -n $arr',
	'property_get -n $arr[0]',
	'property_get -n $arr[1]',
	'eval -- ' . base64_encode("\$GLOBALS['IDE_EVAL_CACHE']['5e298245-9fc5-47cc-9007-2c9e743e87c3']=\$arr"),
	"property_get -c 1 -n \$IDE_EVAL_CACHE['5e298245-9fc5-47cc-9007-2c9e743e87c3']",
	"property_get -c 1 -n \$IDE_EVAL_CACHE['5e298245-9fc5-47cc-9007-2c9e743e87c3'][0]",
	"property_get -c 1 -n \$IDE_EVAL_CACHE['5e298245-9fc5-47cc-9007-2c9e743e87c3'][1]",
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02053.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug02053.inc" lineno="%d"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 22
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug02053.inc" lineno="22"></xdebug:message></response>

-> property_get -i 4 -n $arr
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$arr" fullname="$arr" type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="0" fullname="$arr[0]" type="object" classname="MyObject" children="1" numchildren="3"></property><property name="1" fullname="$arr[1]" type="object" classname="MyObject" children="1" numchildren="3"></property><property name="2" fullname="$arr[2]" type="object" classname="MyObject" children="1" numchildren="3"></property></property></response>

-> property_get -i 5 -n $arr[0]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$arr[0]" fullname="$arr[0]" type="object" classname="MyObject" children="1" numchildren="3" page="0" pagesize="32"><property name="pub" fullname="$arr[0]-&gt;pub" facet="public" type="int"><![CDATA[1]]></property><property name="prot" fullname="$arr[0]-&gt;prot" facet="protected" type="string" size="5" encoding="base64"><![CDATA[Zmlyc3Q=]]></property><property name="priv" fullname="$arr[0]-&gt;priv" facet="private" type="bool"><![CDATA[1]]></property></property></response>

-> property_get -i 6 -n $arr[1]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$arr[1]" fullname="$arr[1]" type="object" classname="MyObject" children="1" numchildren="3" page="0" pagesize="32"><property name="pub" fullname="$arr[1]-&gt;pub" facet="public" type="int"><![CDATA[2]]></property><property name="prot" fullname="$arr[1]-&gt;prot" facet="protected" type="string" size="6" encoding="base64"><![CDATA[c2Vjb25k]]></property><property name="priv" fullname="$arr[1]-&gt;priv" facet="private" type="bool"><![CDATA[0]]></property></property></response>

-> eval -i 7 -- JEdMT0JBTFNbJ0lERV9FVkFMX0NBQ0hFJ11bJzVlMjk4MjQ1LTlmYzUtNDdjYy05MDA3LTJjOWU3NDNlODdjMyddPSRhcnI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="7"><property type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="0" type="object" classname="MyObject" children="1" numchildren="3"></property><property name="1" type="object" classname="MyObject" children="1" numchildren="3"></property><property name="2" type="object" classname="MyObject" children="1" numchildren="3"></property></property></response>

-> property_get -i 8 -c 1 -n $IDE_EVAL_CACHE['5e298245-9fc5-47cc-9007-2c9e743e87c3']
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;]" fullname="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;]" type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="0" fullname="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;][0]" type="object" classname="MyObject" children="1" numchildren="3"></property><property name="1" fullname="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;][1]" type="object" classname="MyObject" children="1" numchildren="3"></property><property name="2" fullname="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;][2]" type="object" classname="MyObject" children="1" numchildren="3"></property></property></response>

-> property_get -i 9 -c 1 -n $IDE_EVAL_CACHE['5e298245-9fc5-47cc-9007-2c9e743e87c3'][0]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;][0]" fullname="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;][0]" type="object" classname="MyObject" children="1" numchildren="3" page="0" pagesize="32"><property name="pub" fullname="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;][0]-&gt;pub" facet="public" type="int"><![CDATA[1]]></property><property name="prot" fullname="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;][0]-&gt;prot" facet="protected" type="string" size="5" encoding="base64"><![CDATA[Zmlyc3Q=]]></property><property name="priv" fullname="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;][0]-&gt;priv" facet="private" type="bool"><![CDATA[1]]></property></property></response>

-> property_get -i 10 -c 1 -n $IDE_EVAL_CACHE['5e298245-9fc5-47cc-9007-2c9e743e87c3'][1]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="10"><property name="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;][1]" fullname="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;][1]" type="object" classname="MyObject" children="1" numchildren="3" page="0" pagesize="32"><property name="pub" fullname="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;][1]-&gt;pub" facet="public" type="int"><![CDATA[2]]></property><property name="prot" fullname="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;][1]-&gt;prot" facet="protected" type="string" size="6" encoding="base64"><![CDATA[c2Vjb25k]]></property><property name="priv" fullname="$IDE_EVAL_CACHE[&#39;5e298245-9fc5-47cc-9007-2c9e743e87c3&#39;][1]-&gt;priv" facet="private" type="bool"><![CDATA[0]]></property></property></response>

-> detach -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="11" status="stopping" reason="ok"></response>
