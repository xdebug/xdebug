--TEST--
Test for bug #886: Use the same file system protocol for file located inside PHAR in both directions
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext phar; dbgp; slow; !osx');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$dir = dirname(__FILE__);
putenv("XDEBUG_TEST_DIR=$dir");
$pharFile = str_replace('\\', '/', "phar://{$dir}/bug00886.phar");
$filename = dirname(__FILE__) . '/bug00886.inc';

$commands = array(
	'step_into',
	'step_into',
	'step_into',
	'stack_get',
	"breakpoint_set -t line -f {$pharFile}/file1.php -n 6",
	'run',
	"source -f {$pharFile}/file1.php",
	'step_into',
	'stack_get',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00886.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug00886.inc" lineno="2"></xdebug:message></response>

-> step_into -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug00886.inc" lineno="3"></xdebug:message></response>

-> step_into -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="3" status="break" reason="ok"><xdebug:message filename="phar://%s/tests/debugger/bug00886.phar/file1.php" lineno="2"></xdebug:message></response>

-> stack_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="4"><stack where="include" level="0" type="file" filename="phar://%s/tests/debugger/bug00886.phar/file1.php" lineno="2"></stack><stack where="{main}" level="1" type="file" filename="file://bug00886.inc" lineno="3"></stack></response>

-> breakpoint_set -i 5 -t line -f phar://%s/tests/debugger/bug00886.phar/file1.php -n 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="5" id=""></response>

-> run -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="6" status="break" reason="ok"><xdebug:message filename="phar://%s/tests/debugger/bug00886.phar/file1.php" lineno="6"></xdebug:message></response>

-> source -i 7 -f phar://%s/tests/debugger/bug00886.phar/file1.php
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="source" transaction_id="7" encoding="base64"><![CDATA[PD9waHAKaW5jbHVkZSAnZmlsZTIucGhwJzsKCmZ1bmN0aW9uIGZ1bmN0aW9uMSggJGZvbyApCnsKCWVjaG8gc3RybGVuKCAkZm9vICksICJcbiI7CglmdW5jdGlvbjIoICRmb28gKTsKfQo/Pgo=]]></response>

-> step_into -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="8" status="break" reason="ok"><xdebug:message filename="phar://%s/tests/debugger/bug00886.phar/file1.php" lineno="6"></xdebug:message></response>

-> stack_get -i 9
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="9"><stack where="function1" level="0" type="file" filename="phar://%s/tests/debugger/bug00886.phar/file1.php" lineno="6"></stack><stack where="{main}" level="1" type="file" filename="file://bug00886.inc" lineno="4"></stack></response>

-> detach -i 10
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="10" status="stopping" reason="ok"></response>
