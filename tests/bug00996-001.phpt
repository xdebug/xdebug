--TEST--
Test for bug #996: Can't evaluate property of class that extends ArrayObject
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$data = file_get_contents(dirname(__FILE__) . '/bug00996.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 10',
	'breakpoint_set -t line -n 14',
	'run',
	'property_get -n $a',
	'property_get -n $a->b',
	'property_get -n $a->b[0]',
	'property_get -n $a->b[\'c\']',
	'property_get -n $a->b[9]',
	'run',
	'context_get',
	'property_get -n $a[\'f\']',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///%sxdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="3"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> breakpoint_set -i 3 -t line -n 14
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id=""></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="10"></xdebug:message></response>

-> property_get -i 5 -n $a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$a" fullname="$a" type="object" classname="Clazz" children="1" numchildren="2" page="0" pagesize="32"><property name="b" fullname="$a-&gt;b" facet="public" type="array" children="1" numchildren="3"></property><property name="*ArrayObject*storage" fullname="$a-&gt;*ArrayObject*storage" facet="private" type="array" children="0" numchildren="0"></property></property></response>

-> property_get -i 6 -n $a->b
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$a-&gt;b" fullname="$a-&gt;b" type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="0" fullname="$a-&gt;b[0]" type="int"><![CDATA[1]]></property><property name="1" fullname="$a-&gt;b[1]" type="int"><![CDATA[2]]></property><property name="c" fullname="$a-&gt;b[&#39;c&#39;]" type="int"><![CDATA[3]]></property></property></response>

-> property_get -i 7 -n $a->b[0]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$a-&gt;b[0]" fullname="$a-&gt;b[0]" type="int"><![CDATA[1]]></property></response>

-> property_get -i 8 -n $a->b['c']
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$a-&gt;b[&#39;c&#39;]" fullname="$a-&gt;b[&#39;c&#39;]" type="int"><![CDATA[3]]></property></response>

-> property_get -i 9 -n $a->b[9]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9" status="break" reason="ok"><error code="300"><message><![CDATA[can not get property]]></message></error></response>

-> run -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="10" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="14"></xdebug:message></response>

-> context_get -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="11" context="0"><property name="$a" fullname="$a" type="object" classname="Clazz" children="1" numchildren="2" page="0" pagesize="32"><property name="b" fullname="$a-&gt;b" facet="public" type="array" children="1" numchildren="3"></property><property name="*ArrayObject*storage" fullname="$a-&gt;*ArrayObject*storage" facet="private" type="array" children="1" numchildren="1"></property></property></response>

-> property_get -i 12 -n $a['f']
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="12"><property name="$a[&#39;f&#39;]" fullname="$a[&#39;f&#39;]" type="string" size="7" encoding="base64"><![CDATA[Y29va2llcw==]]></property></response>
