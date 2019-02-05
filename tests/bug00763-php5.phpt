--TEST--
Test for bug #763: Access method of not yet defined variable aborts debug session (< PHP 7.0)
--SKIPIF--
<?php
if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); }
if (!version_compare(phpversion(), "7.0", '<')) echo "skip < PHP 7.0 needed\n";
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug00763.inc');

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 3',
	'run',
	'stack_get',
	'step_over',
	'stack_get',
	'step_over',
	'stack_get',
	'step_over',
	'xcmd_get_executable_lines -d 0',
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

-> breakpoint_set -i 2 -t line -n 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="3"></xdebug:message></response>

-> stack_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="4"><stack where="{main}" level="0" type="file" filename="file:///%sxdebug-dbgp-test.php" lineno="3"></stack></response>

-> step_over -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="4"></xdebug:message></response>

-> stack_get -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="6"><stack where="{main}" level="0" type="file" filename="file:///%sxdebug-dbgp-test.php" lineno="4"></stack></response>

-> step_over -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="7" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="6"></xdebug:message></response>

-> stack_get -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="8"><stack where="{main}" level="0" type="file" filename="file:///%sxdebug-dbgp-test.php" lineno="6"></stack></response>

-> step_over -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="9" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="8"></xdebug:message></response>

-> xcmd_get_executable_lines -i 10 -d 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="xcmd_get_executable_lines" transaction_id="10"><xdebug:lines><xdebug:line lineno="2"></xdebug:line><xdebug:line lineno="3"></xdebug:line><xdebug:line lineno="4"></xdebug:line><xdebug:line lineno="6"></xdebug:line><xdebug:line lineno="8"></xdebug:line><xdebug:line lineno="10"></xdebug:line></xdebug:lines></response>

-> detach -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="11" status="stopping" reason="ok"></response>
