--TEST--
Test for bug #990: DBGP: Add notification for notices, warnings and errors
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--INI--
xdebug.remote_enable=1
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug00990-002.inc');

$commands = array(
	'step_into',
	'feature_set -n notify_ok -v 1',
	'step_into',
	'step_into',
	'detach'
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///tmp/xdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="2"></xdebug:message></response>

-> feature_set -i 2 -n notify_ok -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="notify_ok" success="1"></response>

-> step_into -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="3"></xdebug:message></response>

-> step_into -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<notify xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" name="error" encoding="base64"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="3" type_string="Fatal error"><![CDATA[Uncaught Error: Class 'MyClass' not found in /tmp/xdebug-dbgp-test.php:3
Stack trace:
#0 {main}
  thrown]]></xdebug:message><![CDATA[CkZhdGFsIGVycm9yOiBVbmNhdWdodCBFcnJvcjogQ2xhc3MgJ015Q2xhc3MnIG5vdCBmb3VuZCBpbiAvdG1wL3hkZWJ1Zy1kYmdwLXRlc3QucGhwIG9uIGxpbmUgMwoKRXJyb3I6IENsYXNzICdNeUNsYXNzJyBub3QgZm91bmQgaW4gL3RtcC94ZGVidWctZGJncC10ZXN0LnBocC%s]]></notify>

<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="4" status="stopping" reason="ok"></response>

-> detach -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="5" status="stopping" reason="ok"></response>
