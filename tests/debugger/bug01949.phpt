--TEST--
Test for bug #1949: Xdebug won't show SplDoublyLinkedLists contents
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01949.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 6',
	'run',
	'property_get -n $stack',
	'property_get -n $stack->*SplDoublyLinkedList*flags',
	'property_get -n $stack->*SplDoublyLinkedList*dllist',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01949.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01949.inc" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug01949.inc" lineno="6"></xdebug:message></response>

-> property_get -i 4 -n $stack
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$stack" fullname="$stack" type="object" classname="SplStack" children="1" numchildren="2" page="0" pagesize="32"><property name="*SplDoublyLinkedList*flags" fullname="$stack-&gt;*SplDoublyLinkedList*flags" facet="private" type="int"><![CDATA[6]]></property><property name="*SplDoublyLinkedList*dllist" fullname="$stack-&gt;*SplDoublyLinkedList*dllist" facet="private" type="array" children="1" numchildren="2"></property></property></response>

-> property_get -i 5 -n $stack->*SplDoublyLinkedList*flags
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$stack-&gt;*SplDoublyLinkedList*flags" fullname="$stack-&gt;*SplDoublyLinkedList*flags" type="int"><![CDATA[6]]></property></response>

-> property_get -i 6 -n $stack->*SplDoublyLinkedList*dllist
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$stack-&gt;*SplDoublyLinkedList*dllist" fullname="$stack-&gt;*SplDoublyLinkedList*dllist" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="0" fullname="$stack-&gt;*SplDoublyLinkedList*dllist[0]" type="string" size="2" encoding="base64"><![CDATA[SEk=]]></property><property name="1" fullname="$stack-&gt;*SplDoublyLinkedList*dllist[1]" type="string" size="14" encoding="base64"><![CDATA[ZnJvbSB0aGUgc3RhY2s=]]></property></property></response>

-> detach -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="7" status="stopping" reason="ok"></response>
