--TEST--
Test for bug #1586: error_reporting evaluation
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01586.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 7',
	'breakpoint_set -t line -n 12',
	'run',
	'eval -- JEdMT0JBTFNbJ0lERV9FVkFMX0NBQ0hFJ11bJzkwZmI4NjBkLTgwOGUtNDRmOC1hMzk2LTM0N2RiZDkwNjU3OCddPWVycm9yX3JlcG9ydGluZygpOw==',
	'run',
	'eval -- JEdMT0JBTFNbJ0lERV9FVkFMX0NBQ0hFJ11bJzkwZmI4NjBkLTgwOGUtNDRmOC1hMzk2LTM0N2RiZDkwNjU3OCddPWVycm9yX3JlcG9ydGluZygpOw==',
	'detach',
);

dbgpRunFile( $filename, $commands, [ 'error_reporting' => E_ALL & ~E_STRICT ] );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file:///%s" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%s" lineno="3"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> breakpoint_set -i 3 -t line -n 12
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id=""></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file:///%s" lineno="7"></xdebug:message></response>

-> eval -i 5 -- JEdMT0JBTFNbJ0lERV9FVkFMX0NBQ0hFJ11bJzkwZmI4NjBkLTgwOGUtNDRmOC1hMzk2LTM0N2RiZDkwNjU3OCddPWVycm9yX3JlcG9ydGluZygpOw==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="5"><property type="int"><![CDATA[30719]]></property></response>

-> run -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="6" status="break" reason="ok"><xdebug:message filename="file:///%s" lineno="12"></xdebug:message></response>

-> eval -i 7 -- JEdMT0JBTFNbJ0lERV9FVkFMX0NBQ0hFJ11bJzkwZmI4NjBkLTgwOGUtNDRmOC1hMzk2LTM0N2RiZDkwNjU3OCddPWVycm9yX3JlcG9ydGluZygpOw==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="7"><property type="int"><![CDATA[32767]]></property></response>

-> detach -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="8" status="stopping" reason="ok"></response>
