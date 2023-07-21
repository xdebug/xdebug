--TEST--
Test for bug #2049: Eval with 'namespace' broken code causes exception
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02049.inc';

$commands = array(
	'feature_set -n resolved_breakpoints -v 1',
	'feature_set -n notify_ok -v 1',
	'feature_set -n extended_properties -v 1',
	'run',
	'stack_get',
	'context_names -d 0',
	'context_get -d 0 -c 0',
	'eval -- bmFtZXNwYWNl',
	'context_names -d 0',
	'context_get -d 0 -c 0',
	'step_over',
	'stack_get',
	'context_names -d 0',
	'context_get -d 0 -c 0',
	'step_over',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02049.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n resolved_breakpoints -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="resolved_breakpoints" success="1"></response>

-> feature_set -i 2 -n notify_ok -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="notify_ok" success="1"></response>

-> feature_set -i 3 -n extended_properties -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="3" feature="extended_properties" success="1"></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://bug02049.inc" lineno="5"></xdebug:message></response>

-> stack_get -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="5"><stack where="{main}" level="0" type="file" filename="file://bug02049.inc" lineno="5"></stack></response>

-> context_names -i 6 -d 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_names" transaction_id="6"><context name="Locals" id="0"></context><context name="Superglobals" id="1"></context><context name="User defined constants" id="2"></context></response>

-> context_get -i 7 -d 0 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="7" context="0"></response>

-> eval -i 8 -- bmFtZXNwYWNl
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="eval" transaction_id="8" status="break" reason="ok"><error code="206"><message><![CDATA[error evaluating code: %s]]></message></error></response>

-> context_names -i 9 -d 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_names" transaction_id="9"><context name="Locals" id="0"></context><context name="Superglobals" id="1"></context><context name="User defined constants" id="2"></context></response>

-> context_get -i 10 -d 0 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="10" context="0"></response>

-> step_over -i 11
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="11" status="break" reason="ok"><xdebug:message filename="file://bug02049.inc" lineno="6"></xdebug:message></response>

-> stack_get -i 12
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="12"><stack where="{main}" level="0" type="file" filename="file://bug02049.inc" lineno="6"></stack></response>

-> context_names -i 13 -d 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_names" transaction_id="13"><context name="Locals" id="0"></context><context name="Superglobals" id="1"></context><context name="User defined constants" id="2"></context></response>

-> context_get -i 14 -d 0 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="14" context="0"></response>

-> step_over -i 15
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="15" status="break" reason="ok"><xdebug:message filename="file://bug02049.inc" lineno="7"></xdebug:message></response>

-> detach -i 16
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="16" status="stopping" reason="ok"></response>
