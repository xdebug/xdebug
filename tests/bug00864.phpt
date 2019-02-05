--TEST--
Test for bug #864: Not possible to inspect ArrayIterator instances with Xdebug
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug00864.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 13',
	'run',
	'property_get -n $a',
	'property_get -n $a->*ArrayIterator*storage',
	'property_get -n $a->*ArrayIterator*storage[2]',
	'detach',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///%sxdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="%r(3|2)%r"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 13
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="13"></xdebug:message></response>

-> property_get -i 4 -n $a
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$a" fullname="$a" type="object" classname="A" children="1" numchildren="2" page="0" pagesize="32"><property name="aa" fullname="$a-&gt;aa" facet="protected" type="int"><![CDATA[302]]></property><property name="*ArrayIterator*storage" fullname="$a-&gt;*ArrayIterator*storage" facet="private" type="array" children="1" numchildren="4"></property></property></response>

-> property_get -i 5 -n $a->*ArrayIterator*storage
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$a-&gt;*ArrayIterator*storage" fullname="$a-&gt;*ArrayIterator*storage" type="array" children="1" numchildren="4" page="0" pagesize="32"><property name="0" fullname="$a-&gt;*ArrayIterator*storage[0]" type="int"><![CDATA[2]]></property><property name="1" fullname="$a-&gt;*ArrayIterator*storage[1]" type="int"><![CDATA[3]]></property><property name="2" fullname="$a-&gt;*ArrayIterator*storage[2]" type="int"><![CDATA[4]]></property><property name="3" fullname="$a-&gt;*ArrayIterator*storage[3]" type="int"><![CDATA[5]]></property></property></response>

-> property_get -i 6 -n $a->*ArrayIterator*storage[2]
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$a-&gt;*ArrayIterator*storage[2]" fullname="$a-&gt;*ArrayIterator*storage[2]" type="int"><![CDATA[4]]></property></response>

-> detach -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="7" status="stopping" reason="ok"></response>
