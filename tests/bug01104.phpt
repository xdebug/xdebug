--TEST--
Test for bug #1104: "Notice: Corrupt member variable name" on 1-character static property
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug01104.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 8',
	'run',
	'context_get',
	'property_get -n $d',
	'feature_set -n max_depth -v 2',
	'property_get -n $d',
	'property_get -n $d::d',
	'detach',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///%sxdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="8"></xdebug:message></response>

-> context_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property name="$d" fullname="$d" type="object" classname="DemoClass" children="1" numchildren="1" page="0" pagesize="32"><property name="d" fullname="$d::d" facet="static protected" type="array" children="1" numchildren="2"></property></property></response>

-> property_get -i 5 -n $d
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$d" fullname="$d" type="object" classname="DemoClass" children="1" numchildren="1" page="0" pagesize="32"><property name="d" fullname="$d::d" facet="static protected" type="array" children="1" numchildren="2"></property></property></response>

-> feature_set -i 6 -n max_depth -v 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="6" feature="max_depth" success="1"></response>

-> property_get -i 7 -n $d
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$d" fullname="$d" type="object" classname="DemoClass" children="1" numchildren="1" page="0" pagesize="32"><property name="d" fullname="$d::d" facet="static protected" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="0" fullname="$d::d[0]" type="int"><![CDATA[42]]></property><property name="1" fullname="$d::d[1]" type="string" size="3" encoding="base64"><![CDATA[Zm9v]]></property></property></property></response>

-> property_get -i 8 -n $d::d
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$d::d" fullname="$d::d" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="0" fullname="$d::d[0]" type="int"><![CDATA[42]]></property><property name="1" fullname="$d::d[1]" type="string" size="3" encoding="base64"><![CDATA[Zm9v]]></property></property></response>

-> detach -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="9" status="stopping" reason="ok"></response>
