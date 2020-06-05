--TEST--
Test for bug #1586: error_reporting evaluation
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug01586.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 7",
	"breakpoint_set -t line -f file://{$filename} -n 12",
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
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01586.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug01586.inc -n 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id=""></response>

-> breakpoint_set -i 2 -t line -f file://bug01586.inc -n 12
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug01586.inc" lineno="7"></xdebug:message></response>

-> eval -i 4 -- JEdMT0JBTFNbJ0lERV9FVkFMX0NBQ0hFJ11bJzkwZmI4NjBkLTgwOGUtNDRmOC1hMzk2LTM0N2RiZDkwNjU3OCddPWVycm9yX3JlcG9ydGluZygpOw==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="4"><property type="int"><![CDATA[30719]]></property></response>

-> run -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file://bug01586.inc" lineno="12"></xdebug:message></response>

-> eval -i 6 -- JEdMT0JBTFNbJ0lERV9FVkFMX0NBQ0hFJ11bJzkwZmI4NjBkLTgwOGUtNDRmOC1hMzk2LTM0N2RiZDkwNjU3OCddPWVycm9yX3JlcG9ydGluZygpOw==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="6"><property type="int"><![CDATA[32767]]></property></response>

-> detach -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="7" status="stopping" reason="ok"></response>
