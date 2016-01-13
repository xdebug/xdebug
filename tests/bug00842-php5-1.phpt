--TEST--
Test for bug #842: Can't debug conditional statements without a block (TS1a (< PHP 7.0))
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
<?php if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n"; ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug00842.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 15',
	'breakpoint_set -t line -n 17',
	'run',
	'stack_get',
	'step_into',
	'stack_get',
	'step_into',
	'step_into',
	'stack_get',
	'detach',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///%sxdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-%d by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 15
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> breakpoint_set -i 3 -t line -n 17
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id=""></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="15"></xdebug:message></response>

-> stack_get -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="5"><stack where="{main}" level="0" type="file" filename="file:///%sxdebug-dbgp-test.php" lineno="15"></stack></response>

-> step_into -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="6" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="12"></xdebug:message></response>

-> stack_get -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="7"><stack where="dummy2" level="0" type="file" filename="file:///%sxdebug-dbgp-test.php" lineno="12"></stack><stack where="{main}" level="1" type="file" filename="file:///%sxdebug-dbgp-test.php" lineno="17"></stack></response>

-> step_into -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="8" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="13"></xdebug:message></response>

-> step_into -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="9" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="18"></xdebug:message></response>

-> stack_get -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="10"><stack where="{main}" level="0" type="file" filename="file:///%sxdebug-dbgp-test.php" lineno="18"></stack></response>

-> detach -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="11" status="stopping" reason="ok"></response>
