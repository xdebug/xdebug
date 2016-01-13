--TEST--
Test for bug #530: Xdebug returns properties out of page if there are less than max_children properties
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug00530.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 12',
	'run',
	'property_get -n a',
	'property_get -n this',
	'property_get -n this -p 0',
	'property_get -n this -p 1',
	'detach'
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///%sxdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2%d by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 12
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="12"></xdebug:message></response>

-> property_get -i 4 -n a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$a" fullname="$a" type="string" size="4" encoding="base64"><![CDATA[Z29nbw==]]></property></response>

-> property_get -i 5 -n this
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$this" fullname="$this" type="object" classname="MyClass" children="1" numchildren="2" page="0" pagesize="32"><property name="a" fullname="$this-&gt;a" facet="private" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property><property name="b" fullname="$this-&gt;b" facet="private" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property></property></response>

-> property_get -i 6 -n this -p 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$this" fullname="$this" type="object" classname="MyClass" children="1" numchildren="2" page="0" pagesize="32"><property name="a" fullname="$this-&gt;a" facet="private" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property><property name="b" fullname="$this-&gt;b" facet="private" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property></property></response>

-> property_get -i 7 -n this -p 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$this" fullname="$this" type="object" classname="MyClass" children="1" numchildren="2" page="1" pagesize="32"></property></response>

-> detach -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="8" status="stopping" reason="ok"></response>
