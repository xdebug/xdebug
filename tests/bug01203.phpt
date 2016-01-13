--TEST--
Test for bug #1203: Accessing static property of a class that has no static properties crashes while remote debugging
--SKIPIF--
<?php if (!version_compare(phpversion(), "7.0", '>=')) echo "skip >= PHP 7.0 needed\n"; ?>
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug01202.inc');

$commands = array(
	'step_into',
	'step_into',
	'context_get',
	'property_get -n $a',
	'property_get -n $a::a',
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

-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="9"></xdebug:message></response>

-> context_get -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="3" context="0"><property name="$a" fullname="$a" type="object" classname="class@anonymous" children="1" numchildren="1" page="0" pagesize="32"><property name="a" fullname="$a-&gt;a" facet="public" type="int"><![CDATA[42]]></property></property></response>

-> property_get -i 4 -n $a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$a" fullname="$a" type="object" classname="class@anonymous" children="1" numchildren="1" page="0" pagesize="32"><property name="a" fullname="$a-&gt;a" facet="public" type="int"><![CDATA[42]]></property></property></response>

-> property_get -i 5 -n $a::a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5" status="break" reason="ok"><error code="300"><message><![CDATA[can not get property]]></message></error></response>

-> detach -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="6" status="stopping" reason="ok"></response>
