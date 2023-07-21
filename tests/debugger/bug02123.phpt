--TEST--
Test for bug #2123: Add warning in log and diagnositics information when a breakpoint is set on a non-existing file
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; !win');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02123.inc';

$commands = array(
	'breakpoint_set -t line -f /var/www/elephpants-are-cool/does-not-exist -n 1',
	'run',
	'detach',
);

$xdebugLogFileName = sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'remote-log-2123.txt';
@unlink( $xdebugLogFileName );

dbgpRunFile( $filename, $commands, [ 'xdebug.log' => $xdebugLogFileName, 'xdebug.log_level' => 3 ], [ 'show-stdout' => true ] );

echo file_get_contents( $xdebugLogFileName );
@unlink( $xdebugLogFileName );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02123.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f /var/www/elephpants-are-cool/does-not-exist -n 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="{{PID}}0001"></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="stopping" reason="ok"></response>

-> detach -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="3" status="stopping" reason="ok"></response>

%A[Step Debug] WARN: Breakpoint file name does not exist: /var/www/elephpants-are-cool/does-not-exist (No such file or directory).%A

[%d] Log opened at %s
[%d] [Step Debug] WARN: Breakpoint file name does not exist: /var/www/elephpants-are-cool/does-not-exist (No such file or directory).
[%d] Log closed at %s
