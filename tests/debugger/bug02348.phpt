--TEST--
Test for bug #2348: Breakpoints in property hooks are not resolved
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.4; dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02348.inc';

$commands = array(
	'feature_set -n resolved_breakpoints -v 1',
	"breakpoint_set -t line -f file://{$filename} -n 7",
	'run',
	'detach',
);

$xdebugLogFileName = sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'remote-log-2343.txt';
@unlink( $xdebugLogFileName );

dbgpRunFile( $filename, $commands, [ 'xdebug.log' => $xdebugLogFileName, 'xdebug.log_level' => 10 ] );

echo file_get_contents( $xdebugLogFileName );
@unlink( $xdebugLogFileName );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02348.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n resolved_breakpoints -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="resolved_breakpoints" success="1"></response>

-> breakpoint_set -i 2 -t line -f file://bug02348.inc -n 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="{{PID}}0001" resolved="resolved"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://bug02348.inc" lineno="7"></xdebug:message></response>

-> detach -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="4" status="stopping" reason="ok"></response>

[%d] Log opened at %s
[%d] [Step Debug] INFO: Connecting to configured address/port: 127.0.0.1:%d.
[%d] [Step Debug] INFO: Connected to debugging client: 127.0.0.1:%d (through xdebug.client_host/xdebug.client_port).
[%d] [Step Debug] -> <init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="%sbug02348.inc" language="PHP" xdebug:language_version="%s" protocol_version="1.0" appid="%d"><engine version="%s"><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[%s]]></copyright></init>

[%d] [Step Debug] <- feature_set -i 1 -n resolved_breakpoints -v 1
[%d] [Step Debug] -> <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="resolved_breakpoints" success="1"></response>

[%d] [Step Debug] <- breakpoint_set -i 2 -t line -f %sbug02348.inc -n 7
[%d] [Step Debug] DEBUG: R: Line number (7) out of range (3-3).
[%d] [Step Debug] DEBUG: R: Line number (7) out of range (9-9).
[%d] [Step Debug] DEBUG: R: Line number (7) in smallest range of range (6-8).
[%d] [Step Debug] DEBUG: F: Breakpoint line (7) found in set of executable lines.
[%d] [Step Debug] -> <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id="%d" resolved="resolved"></response>

[%d] [Step Debug] <- run -i 3
[%d] [Step Debug] DEBUG: Setting 'has_line_breakpoints on {main} (%sbug02348.inc:0)
[%d] [Step Debug] DEBUG: Checking whether to break on %sbug02348.inc:7.
[%d] [Step Debug] DEBUG: I: Current location: %sbug02348.inc:2.
[%d] [Step Debug] DEBUG: I: Matching breakpoint '%sbug02348.inc:7' against location '%sbug02348.inc:2'.
[%d] [Step Debug] DEBUG: R: Line number (2) doesn't match with breakpoint (7).
[%d] [Step Debug] DEBUG: Checking whether to break on %sbug02348.inc:7.
[%d] [Step Debug] DEBUG: I: Current location: %sbug02348.inc:13.
[%d] [Step Debug] DEBUG: I: Matching breakpoint '%sbug02348.inc:7' against location '%sbug02348.inc:13'.
[%d] [Step Debug] DEBUG: R: Line number (13) doesn't match with breakpoint (7).
[%d] [Step Debug] DEBUG: Setting 'has_line_breakpoints on $datetime::get (%sbug02348.inc:13)
[%d] [Step Debug] DEBUG: Checking whether to break on %sbug02348.inc:7.
[%d] [Step Debug] DEBUG: I: Current location: %sbug02348.inc:7.
[%d] [Step Debug] DEBUG: I: Matching breakpoint '%sbug02348.inc:7' against location '%sbug02348.inc:7'.
[%d] [Step Debug] DEBUG: F: File names match (%sbug02348.inc).
[%d] [Step Debug] -> <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="%sbug02348.inc" lineno="7"></xdebug:message></response>

[%d] [Step Debug] <- detach -i 4
[%d] [Step Debug] -> <response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="4" status="stopping" reason="ok"></response>

[%d] Log closed at %s
