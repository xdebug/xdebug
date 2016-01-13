--TEST--
Test for bug #619: Xdebug does NOT show "private" properties defined in parent classes
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug00619.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 16',
	'run',
	'stack_get',
	'property_get -n this',
	'property_get -n $this',
	'property_get -n $this->*P*private_prop',
	'detach'
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///%sxdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-%d by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 16
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="16"></xdebug:message></response>

-> stack_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="4"><stack where="C-&gt;__construct" level="0" type="file" filename="file:///%sxdebug-dbgp-test.php" lineno="16"></stack><stack where="{main}" level="1" type="file" filename="file://%sxdebug-dbgp-test.php" lineno="20"></stack></response>

-> property_get -i 5 -n this
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$this" fullname="$this" type="object" classname="C" children="1" numchildren="3" page="0" pagesize="32"><property name="*P*private_prop" fullname="$this-&gt;*P*private_prop" facet="private" type="string" size="0" encoding="base64"><![CDATA[]]></property><property name="protected_prop" fullname="$this-&gt;protected_prop" facet="protected" type="string" size="0" encoding="base64"><![CDATA[]]></property><property name="public_prop" fullname="$this-&gt;public_prop" facet="public" type="string" size="0" encoding="base64"><![CDATA[]]></property></property></response>

-> property_get -i 6 -n $this
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$this" fullname="$this" type="object" classname="C" children="1" numchildren="3" page="0" pagesize="32"><property name="*P*private_prop" fullname="$this-&gt;*P*private_prop" facet="private" type="string" size="0" encoding="base64"><![CDATA[]]></property><property name="protected_prop" fullname="$this-&gt;protected_prop" facet="protected" type="string" size="0" encoding="base64"><![CDATA[]]></property><property name="public_prop" fullname="$this-&gt;public_prop" facet="public" type="string" size="0" encoding="base64"><![CDATA[]]></property></property></response>

-> property_get -i 7 -n $this->*P*private_prop
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$this-&gt;*P*private_prop" fullname="$this-&gt;*P*private_prop" type="string" size="0" encoding="base64"><![CDATA[]]></property></response>

-> detach -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="8" status="stopping" reason="ok"></response>
