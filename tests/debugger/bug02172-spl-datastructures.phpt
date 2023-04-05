--TEST--
Test for bug #2172: Support SplDoublyLinkedList and SplPriorityQueue
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug02172-spl-datastructures.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 14",
	'run',
	'property_get -n $dll',
	'property_get -n $dll->dllist',
	'property_get -n $dll->dllist[3]',
	'property_get -n $priorityQueue',
	'property_get -n $priorityQueue->heap',
	'property_get -n $priorityQueue->heap[1]',
	'property_get -n $priorityQueue->heap[1][\'data\']',
	'property_get -n $priorityQueue->heap[1][\'priority\']',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02172-spl-datastructures.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug02172-spl-datastructures.inc -n 14
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="{{PID}}0001"></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug02172-spl-datastructures.inc" lineno="14"></xdebug:message></response>

-> property_get -i 3 -n $dll
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="3"><property name="$dll" fullname="$dll" type="object" classname="SplDoublyLinkedList" children="1" numchildren="2" page="0" pagesize="32"><property name="flags" fullname="$dll-&gt;flags" facet="private" type="int"><![CDATA[0]]></property><property name="dllist" fullname="$dll-&gt;dllist" facet="private" type="array" children="1" numchildren="4"></property></property></response>

-> property_get -i 4 -n $dll->dllist
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$dll-&gt;dllist" fullname="$dll-&gt;dllist" type="array" children="1" numchildren="4" page="0" pagesize="32"><property name="0" fullname="$dll-&gt;dllist[0]" type="string" size="1" encoding="base64"><![CDATA[YQ==]]></property><property name="1" fullname="$dll-&gt;dllist[1]" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property><property name="2" fullname="$dll-&gt;dllist[2]" type="string" size="1" encoding="base64"><![CDATA[Yw==]]></property><property name="3" fullname="$dll-&gt;dllist[3]" type="string" size="1" encoding="base64"><![CDATA[ZA==]]></property></property></response>

-> property_get -i 5 -n $dll->dllist[3]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$dll-&gt;dllist[3]" fullname="$dll-&gt;dllist[3]" type="string" size="1" encoding="base64"><![CDATA[ZA==]]></property></response>

-> property_get -i 6 -n $priorityQueue
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$priorityQueue" fullname="$priorityQueue" type="object" classname="SplPriorityQueue" children="1" numchildren="3" page="0" pagesize="32"><property name="flags" fullname="$priorityQueue-&gt;flags" facet="private" type="int"><![CDATA[1]]></property><property name="isCorrupted" fullname="$priorityQueue-&gt;isCorrupted" facet="private" type="bool"><![CDATA[0]]></property><property name="heap" fullname="$priorityQueue-&gt;heap" facet="private" type="array" children="1" numchildren="4"></property></property></response>

-> property_get -i 7 -n $priorityQueue->heap
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$priorityQueue-&gt;heap" fullname="$priorityQueue-&gt;heap" type="array" children="1" numchildren="4" page="0" pagesize="32"><property name="0" fullname="$priorityQueue-&gt;heap[0]" type="array" children="1" numchildren="2"></property><property name="1" fullname="$priorityQueue-&gt;heap[1]" type="array" children="1" numchildren="2"></property><property name="2" fullname="$priorityQueue-&gt;heap[2]" type="array" children="1" numchildren="2"></property><property name="3" fullname="$priorityQueue-&gt;heap[3]" type="array" children="1" numchildren="2"></property></property></response>

-> property_get -i 8 -n $priorityQueue->heap[1]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$priorityQueue-&gt;heap[1]" fullname="$priorityQueue-&gt;heap[1]" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="data" fullname="$priorityQueue-&gt;heap[1][&quot;data&quot;]" type="string" size="1" encoding="base64"><![CDATA[QQ==]]></property><property name="priority" fullname="$priorityQueue-&gt;heap[1][&quot;priority&quot;]" type="int"><![CDATA[3]]></property></property></response>

-> property_get -i 9 -n $priorityQueue->heap[1]['data']
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="$priorityQueue-&gt;heap[1][&#39;data&#39;]" fullname="$priorityQueue-&gt;heap[1][&#39;data&#39;]" type="string" size="1" encoding="base64"><![CDATA[QQ==]]></property></response>

-> property_get -i 10 -n $priorityQueue->heap[1]['priority']
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="10"><property name="$priorityQueue-&gt;heap[1][&#39;priority&#39;]" fullname="$priorityQueue-&gt;heap[1][&#39;priority&#39;]" type="int"><![CDATA[3]]></property></response>

-> detach -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="11" status="stopping" reason="ok"></response>
