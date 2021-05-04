--TEST--
DBGP: return breakpoints with method
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/dbgp-breakpoint-call-method.inc';

$commands = array(
	'feature_set -n breakpoint_details -v 1',
	'breakpoint_set -t return -m breaking::staticMe',
	'breakpoint_set -t return -m breaking::onMe',
	'run',
	'run',
	'run',
	'run',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://dbgp-breakpoint-call-method.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n breakpoint_details -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="breakpoint_details" success="1"></response>

-> breakpoint_set -i 2 -t return -m breaking::staticMe
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> breakpoint_set -i 3 -t return -m breaking::onMe
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id=""></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-call-method.inc" lineno="15"></xdebug:message><breakpoint type="return" function="breaking::staticMe" state="enabled" hit_count="1" hit_value="0" id=""></breakpoint></response>

-> run -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-call-method.inc" lineno="18"></xdebug:message><breakpoint type="return" function="breaking::onMe" state="enabled" hit_count="1" hit_value="0" id=""></breakpoint></response>

-> run -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="6" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-call-method.inc" lineno="19"></xdebug:message><breakpoint type="return" function="breaking::staticMe" state="enabled" hit_count="2" hit_value="0" id=""></breakpoint></response>

-> run -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="7" status="break" reason="ok"><xdebug:message filename="file://dbgp-breakpoint-call-method.inc" lineno="20"></xdebug:message><breakpoint type="return" function="breaking::onMe" state="enabled" hit_count="2" hit_value="0" id=""></breakpoint></response>

-> detach -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="8" status="stopping" reason="ok"></response>
