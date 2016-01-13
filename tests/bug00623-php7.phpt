--TEST--
Test for bug #623: Static properties of a class can be evaluated only with difficulty (>= PHP 7.0)
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
<?php if (!version_compare(phpversion(), "7.0", '>=')) echo "skip >= PHP 7.0 needed\n"; ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug00623.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 12',
	'breakpoint_set -t line -n 24',
	'run',
	'context_get',
	'property_get -n ::',
	'property_get -n ::*testclassDaddy*daddyPriv',
	'property_get -n ::nameProt::*testclassDaddy*daddyPriv',
	'property_get -n ::*testclassDaddy*daddyPriv[3]',
	'run',
	'context_get',
	'property_get -n t',
	'property_get -n $t',
	'property_get -n $t::',
	'property_get -n $t::nameProt',
	'detach',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///%sxdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-%d by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="3"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 12
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> breakpoint_set -i 3 -t line -n 24
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id=""></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="12"></xdebug:message></response>

-> context_get -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="5" context="0"><property name="::" fullname="::" type="object" classname="testclass" children="1" numchildren="3"><property name="::nameProt" fullname="::nameProt" type="object" classname="testclass" children="1" numchildren="3" page="0" pagesize="32" facet="static protected"><property name="nameProt" fullname="::nameProt::nameProt" facet="static protected" type="object" classname="testclass" children="1" numchildren="3"></property><property name="namePriv" fullname="::nameProt::namePriv" facet="static private" type="null"></property><property name="*testclassDaddy*daddyPriv" fullname="::nameProt::*testclassDaddy*daddyPriv" facet="static private" type="array" children="1" numchildren="4"></property></property><property name="::namePriv" fullname="::namePriv" type="null" facet="static private"></property><property name="::*testclassDaddy*daddyPriv" fullname="::*testclassDaddy*daddyPriv" type="array" children="1" numchildren="4" page="0" pagesize="32" facet="static private"><property name="0" fullname="::*testclassDaddy*daddyPriv[0]" type="int"><![CDATA[1]]></property><property name="1" fullname="::*testclassDaddy*daddyPriv[1]" type="int"><![CDATA[2]]></property><property name="2" fullname="::*testclassDaddy*daddyPriv[2]" type="int"><![CDATA[3]]></property><property name="3" fullname="::*testclassDaddy*daddyPriv[3]" type="int"><![CDATA[9]]></property></property></property></response>

-> property_get -i 6 -n ::
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6" status="break" reason="ok"><error code="300"><message><![CDATA[can not get property]]></message></error></response>

-> property_get -i 7 -n ::*testclassDaddy*daddyPriv
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="::*testclassDaddy*daddyPriv" fullname="::*testclassDaddy*daddyPriv" type="array" children="1" numchildren="4" page="0" pagesize="32"><property name="0" fullname="::*testclassDaddy*daddyPriv[0]" type="int"><![CDATA[1]]></property><property name="1" fullname="::*testclassDaddy*daddyPriv[1]" type="int"><![CDATA[2]]></property><property name="2" fullname="::*testclassDaddy*daddyPriv[2]" type="int"><![CDATA[3]]></property><property name="3" fullname="::*testclassDaddy*daddyPriv[3]" type="int"><![CDATA[9]]></property></property></response>

-> property_get -i 8 -n ::nameProt::*testclassDaddy*daddyPriv
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="::nameProt::*testclassDaddy*daddyPriv" fullname="::nameProt::*testclassDaddy*daddyPriv" type="array" children="1" numchildren="4" page="0" pagesize="32"><property name="0" fullname="::nameProt::*testclassDaddy*daddyPriv[0]" type="int"><![CDATA[1]]></property><property name="1" fullname="::nameProt::*testclassDaddy*daddyPriv[1]" type="int"><![CDATA[2]]></property><property name="2" fullname="::nameProt::*testclassDaddy*daddyPriv[2]" type="int"><![CDATA[3]]></property><property name="3" fullname="::nameProt::*testclassDaddy*daddyPriv[3]" type="int"><![CDATA[9]]></property></property></response>

-> property_get -i 9 -n ::*testclassDaddy*daddyPriv[3]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="::*testclassDaddy*daddyPriv[3]" fullname="::*testclassDaddy*daddyPriv[3]" type="int"><![CDATA[9]]></property></response>

-> run -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="10" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="24"></xdebug:message></response>

-> context_get -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="11" context="0"><property name="$t" fullname="$t" type="object" classname="testclass" children="1" numchildren="3" page="0" pagesize="32"><property name="nameProt" fullname="$t::nameProt" facet="static protected" type="object" classname="testclass" children="1" numchildren="3"></property><property name="namePriv" fullname="$t::namePriv" facet="static private" type="null"></property><property name="*testclassDaddy*daddyPriv" fullname="$t::*testclassDaddy*daddyPriv" facet="static private" type="array" children="1" numchildren="4"></property></property></response>

-> property_get -i 12 -n t
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="12"><property name="$t" fullname="$t" type="object" classname="testclass" children="1" numchildren="3" page="0" pagesize="32"><property name="nameProt" fullname="$t::nameProt" facet="static protected" type="object" classname="testclass" children="1" numchildren="3"></property><property name="namePriv" fullname="$t::namePriv" facet="static private" type="null"></property><property name="*testclassDaddy*daddyPriv" fullname="$t::*testclassDaddy*daddyPriv" facet="static private" type="array" children="1" numchildren="4"></property></property></response>

-> property_get -i 13 -n $t
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="13"><property name="$t" fullname="$t" type="object" classname="testclass" children="1" numchildren="3" page="0" pagesize="32"><property name="nameProt" fullname="$t::nameProt" facet="static protected" type="object" classname="testclass" children="1" numchildren="3"></property><property name="namePriv" fullname="$t::namePriv" facet="static private" type="null"></property><property name="*testclassDaddy*daddyPriv" fullname="$t::*testclassDaddy*daddyPriv" facet="static private" type="array" children="1" numchildren="4"></property></property></response>

-> property_get -i 14 -n $t::
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="14"><property name="$t" fullname="$t" type="object" classname="testclass" children="1" numchildren="3" page="0" pagesize="32"><property name="nameProt" fullname="$t::nameProt" facet="static protected" type="object" classname="testclass" children="1" numchildren="3"></property><property name="namePriv" fullname="$t::namePriv" facet="static private" type="null"></property><property name="*testclassDaddy*daddyPriv" fullname="$t::*testclassDaddy*daddyPriv" facet="static private" type="array" children="1" numchildren="4"></property></property></response>

-> property_get -i 15 -n $t::nameProt
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="15"><property name="$t::nameProt" fullname="$t::nameProt" type="object" classname="testclass" children="1" numchildren="3" page="0" pagesize="32"><property name="nameProt" fullname="$t::nameProt::nameProt" facet="static protected" type="object" classname="testclass" children="1" numchildren="3"></property><property name="namePriv" fullname="$t::nameProt::namePriv" facet="static private" type="null"></property><property name="*testclassDaddy*daddyPriv" fullname="$t::nameProt::*testclassDaddy*daddyPriv" facet="static private" type="array" children="1" numchildren="4"></property></property></response>

-> detach -i 16
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="16" status="stopping" reason="ok"></response>
