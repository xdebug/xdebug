--TEST--
DBGP: Enum properties
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; PHP >= 8.1');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$filename = dirname(__FILE__) . '/dbgp-property-enum.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 23',
	'run',
	'property_set -n $eur->value -- ' . base64_encode('"EURO"'),
	'context_get',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://dbgp-property-enum.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://dbgp-property-enum.inc" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 23
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://dbgp-property-enum.inc" lineno="23"></xdebug:message></response>

-> property_set -i 4 -n $eur->value -- IkVVUk8i
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="4" success="0"></response>

-> context_get -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="5" context="0"><property name="$eur" fullname="$eur" type="object" facet="enum" classname="Currency" children="1" numchildren="2" page="0" pagesize="32"><property name="name" fullname="$eur-&gt;name" facet="public" type="string" size="3" encoding="base64"><![CDATA[RVVS]]></property><property name="value" fullname="$eur-&gt;value" facet="public" type="string" size="3" encoding="base64"><![CDATA[4oKs]]></property></property><property name="$lang" fullname="$lang" type="object" facet="enum" classname="Language" children="1" numchildren="1" page="0" pagesize="32"><property name="name" fullname="$lang-&gt;name" facet="public" type="string" size="9" encoding="base64"><![CDATA[R8OgaWRobGln]]></property></property><property name="$time" fullname="$time" type="object" facet="enum" classname="Unit" children="1" numchildren="2" page="0" pagesize="32"><property name="name" fullname="$time-&gt;name" facet="public" type="string" size="4" encoding="base64"><![CDATA[SG91cg==]]></property><property name="value" fullname="$time-&gt;value" facet="public" type="int"><![CDATA[3600]]></property></property></response>
