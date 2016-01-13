--TEST--
Test for bug #1181: Derived class with __get gets called on fetching base class private property
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug01181.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 17',
	'run',
	'property_get -n $test->*BaseClass*private',
	'step_into',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///%sxdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="3"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 17
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="17"></xdebug:message></response>

-> property_get -i 4 -n $test->*BaseClass*private
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$test-&gt;*BaseClass*private" fullname="$test-&gt;*BaseClass*private" type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="0" fullname="$test-&gt;*BaseClass*private[0]" type="string" size="1" encoding="base64"><![CDATA[YQ==]]></property><property name="1" fullname="$test-&gt;*BaseClass*private[1]" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property><property name="2" fullname="$test-&gt;*BaseClass*private[2]" type="string" size="1" encoding="base64"><![CDATA[Yw==]]></property></property></response>

-> step_into -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="22"></xdebug:message></response>
