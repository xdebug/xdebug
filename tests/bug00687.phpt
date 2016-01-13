--TEST--
Test for bug #687: Xdebug does not show dynamically defined variable
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug00687.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 4',
	'run',
	'context_get',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///%sxdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-%d by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="4"></xdebug:message></response>

-> context_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property name="$name" fullname="$name" type="string" size="5" encoding="base64"><![CDATA[dmFsdWU=]]></property><property name="$nameHolder" fullname="$nameHolder" type="string" size="4" encoding="base64"><![CDATA[bmFtZQ==]]></property></response>
