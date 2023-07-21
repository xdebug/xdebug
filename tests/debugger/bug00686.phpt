--TEST--
Test for bug #686: Not possible to inspect SplObjectStorage instances with Xdebug
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug00686.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 12',
	'run',
	'property_get -n $storage',
	'property_get -n $storage->storage',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00686.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug00686.inc" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 12
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug00686.inc" lineno="12"></xdebug:message></response>

-> property_get -i 4 -n $storage
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$storage" fullname="$storage" type="object" classname="SplObjectStorage" children="1" numchildren="1" page="0" pagesize="32"><property name="storage" fullname="$storage-&gt;storage" facet="private" type="array" children="1" numchildren="2"></property></property></response>

-> property_get -i 5 -n $storage->storage
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$storage-&gt;storage" fullname="$storage-&gt;storage" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="%s" fullname="$storage-&gt;storage[%s]" type="array" children="1" numchildren="2"></property><property name="%s" fullname="$storage-&gt;storage[%s]" type="array" children="1" numchildren="2"></property></property></response>

-> detach -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="6" status="stopping" reason="ok"></response>
