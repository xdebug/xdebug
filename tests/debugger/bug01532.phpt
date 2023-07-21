--TEST--
Test for bug #1532: SIGABRT when using remote debugging and an error is thrown in eval()
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01532.inc';

$commands = array(
    'feature_set -n notify_ok -v 1',
    'step_into',
    'run',
    'detach'
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01532.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n notify_ok -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="notify_ok" success="1"></response>

-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug01532.inc" lineno="1"></xdebug:message></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<notify xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" name="error"><xdebug:message filename="dbgp://1" lineno="1" type="Notice"><![CDATA[flupp]]></xdebug:message></notify>

<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="stopping" reason="ok"></response>

-> detach -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="4" status="stopping" reason="ok"></response>
