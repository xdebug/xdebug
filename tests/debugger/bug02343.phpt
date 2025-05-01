--TEST--
Test for bug #2343: Virtual properties can not be shown safely
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.4; dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02343.inc';

$commands = array(
	'feature_set -n extended_properties -v 1',
	'feature_set -n max_depth -v 2',
	"breakpoint_set -t line -f file://{$filename} -n 9",
	'breakpoint_set -t exception -x *',
	'run',
	'context_get -c 0',
	'step_into',
	'context_get -c 0',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02343.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n extended_properties -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="extended_properties" success="1"></response>

-> feature_set -i 2 -n max_depth -v 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="max_depth" success="1"></response>

-> breakpoint_set -i 3 -t line -f file://bug02343.inc -n 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id="{{PID}}0001"></response>

-> breakpoint_set -i 4 -t exception -x *
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="4" id="{{PID}}0002"></response>

-> run -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="5" status="break" reason="ok"><xdebug:message filename="file://bug02343.inc" lineno="9"></xdebug:message></response>

-> context_get -i 6 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="6" context="0"><property type="object" children="0" numchildren="0" page="0" pagesize="32"><name encoding="base64"><![CDATA[JHRoaXM=]]></name><fullname encoding="base64"><![CDATA[JHRoaXM=]]></fullname><classname encoding="base64"><![CDATA[%s]]></classname></property></response>

-> step_into -i 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="7" status="break" reason="ok"><xdebug:message filename="file://bug02343.inc" lineno="16"></xdebug:message></response>

-> context_get -i 8 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="8" context="0"><property type="object" children="0" numchildren="0" page="0" pagesize="32"><name encoding="base64"><![CDATA[JGV4YW1wbGU=]]></name><fullname encoding="base64"><![CDATA[JGV4YW1wbGU=]]></fullname><classname encoding="base64"><![CDATA[%s]]></classname></property></response>

-> detach -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="9" status="stopping" reason="ok"></response>
