--TEST--
Test for bug #864: Not possible to inspect ArrayIterator instances with Xdebug
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug00864.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 13",
	'run',
	'property_get -n $a',
	'property_get -n $a->*ArrayIterator*storage',
	'property_get -n $a->*ArrayIterator*storage[2]',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00864.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug00864.inc -n 13
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="{{PID}}0001"></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug00864.inc" lineno="13"></xdebug:message></response>

-> property_get -i 3 -n $a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="3"><property name="$a" fullname="$a" type="object" classname="A" children="1" numchildren="2" page="0" pagesize="32"><property name="aa" fullname="$a-&gt;aa" facet="protected" type="int"><![CDATA[302]]></property><property name="*ArrayIterator*storage" fullname="$a-&gt;*ArrayIterator*storage" facet="private" type="array" children="1" numchildren="4"></property></property></response>

-> property_get -i 4 -n $a->*ArrayIterator*storage
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$a-&gt;*ArrayIterator*storage" fullname="$a-&gt;*ArrayIterator*storage" type="array" children="1" numchildren="4" page="0" pagesize="32"><property name="0" fullname="$a-&gt;*ArrayIterator*storage[0]" type="int"><![CDATA[2]]></property><property name="1" fullname="$a-&gt;*ArrayIterator*storage[1]" type="int"><![CDATA[3]]></property><property name="2" fullname="$a-&gt;*ArrayIterator*storage[2]" type="int"><![CDATA[4]]></property><property name="3" fullname="$a-&gt;*ArrayIterator*storage[3]" type="int"><![CDATA[5]]></property></property></response>

-> property_get -i 5 -n $a->*ArrayIterator*storage[2]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$a-&gt;*ArrayIterator*storage[2]" fullname="$a-&gt;*ArrayIterator*storage[2]" type="int"><![CDATA[4]]></property></response>

-> detach -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="6" status="stopping" reason="ok"></response>
