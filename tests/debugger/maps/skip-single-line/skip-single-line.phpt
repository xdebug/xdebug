--TEST--
DBGP: skip single line in script
--SKIPIF--
<?php
require __DIR__ . '/../../../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require __DIR__ . '/../../dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/skip-single-line.inc';

$xdebugLogFileName = sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'skip-single-line.txt';
@unlink( $xdebugLogFileName );

$commands = array(
	'stdout -c 1',
	'step_into',
	'step_into',
	'run',
	'detach',
);

dbgpRunFile(
	$filename, $commands,
	[
		'xdebug.mode' => 'debug', 'xdebug.start_with_request' => 'yes',
		'xdebug.log' => $xdebugLogFileName, 'xdebug.log_level' => 10,
		'xdebug.path_mapping' => 'yes',
	]
);

echo file_get_contents( $xdebugLogFileName );
@unlink( $xdebugLogFileName );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://skip-single-line.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> stdout -i 1 -c 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stdout" transaction_id="1" success="1"></response>

-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://skip-single-line.inc" lineno="2"></xdebug:message></response>

-> step_into -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<stream xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" type="stdout" encoding="base64"><![CDATA[SGVsbG8=]]></stream>

<?xml version="1.0" encoding="iso-8859-1"?>
<stream xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" type="stdout" encoding="base64"><![CDATA[SG93IGFyZSB5b3U/]]></stream>

<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://skip-single-line.inc" lineno="4"></xdebug:message></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<stream xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" type="stdout" encoding="base64"><![CDATA[SSBhbSB3ZWxs]]></stream>

<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="stopping" reason="ok"></response>

-> detach -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="5" status="stopping" reason="ok"></response>
%A
