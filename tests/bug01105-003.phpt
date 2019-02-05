--TEST--
Test for bug #1105: Setting properties without specifying a type only works in topmost frame
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug01105-003.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 5',
	'run',
	'property_set -n $var -d 0 -- ' . base64_encode('"scope0-modified"'),
	'property_set -n $var -d 1 -- ' . base64_encode('"scope1-modified"'),
	'property_set -n $var -d 2 -- ' . base64_encode('"scope2-modified"'),
	'property_get -n $var -d 0',
	'property_get -n $var -d 1',
	'property_get -n $var -d 2',
	'detach',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///%sxdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="5"></xdebug:message></response>

-> property_set -i 4 -n $var -d 0 -- InNjb3BlMC1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="4" success="1"></response>

-> property_set -i 5 -n $var -d 1 -- InNjb3BlMS1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="5" success="1"></response>

-> property_set -i 6 -n $var -d 2 -- InNjb3BlMi1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="6" success="1"></response>

-> property_get -i 7 -n $var -d 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$var" fullname="$var" type="string" size="15" encoding="base64"><![CDATA[c2NvcGUwLW1vZGlmaWVk]]></property></response>

-> property_get -i 8 -n $var -d 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$var" fullname="$var" type="string" size="15" encoding="base64"><![CDATA[c2NvcGUxLW1vZGlmaWVk]]></property></response>

-> property_get -i 9 -n $var -d 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="$var" fullname="$var" type="string" size="15" encoding="base64"><![CDATA[c2NvcGUyLW1vZGlmaWVk]]></property></response>

-> detach -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="10" status="stopping" reason="ok"></response>
