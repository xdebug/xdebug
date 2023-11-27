--TEST--
Test for bug #2211: File wrapper gets wrong filename location in stack
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02211.inc';

$commands = array(
	'run',
	'stack_get',
	'detach',
);

dbgpRunFile( $filename, $commands, array( 'allow_url_include' => '1' ) );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02211.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> run -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="1" status="break" reason="ok"><xdebug:message filename="data://text/plain;base64,PD9waHAKcmV0dXJuIGZ1bmN0aW9uKCkgewp4ZGVidWdfYnJlYWsoKTsKfTs=" lineno="4"></xdebug:message></response>

-> stack_get -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="2"><stack where="{closure:data://text/plain;base64,PD9waHAKcmV0dXJuIGZ1bmN0aW9uKCkgewp4ZGVidWdfYnJlYWsoKTsKfTs=:2-4}" level="0" type="file" filename="data://text/plain;base64,PD9waHAKcmV0dXJuIGZ1bmN0aW9uKCkgewp4ZGVidWdfYnJlYWsoKTsKfTs=" lineno="4"></stack><stack where="{main}" level="1" type="file" filename="file://bug02211.inc" lineno="6"></stack></response>

-> detach -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="3" status="stopping" reason="ok"></response>
