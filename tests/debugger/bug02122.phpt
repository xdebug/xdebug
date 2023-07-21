--TEST--
Test for bug #2122: Local variables are not available when using start_upon_error
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; !win');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug02122.inc';

$commands = array(
	'context_get -d 0',
	'context_get -d 1',
	'detach',
);

dbgpRunFile( $filename, $commands, [ 'xdebug.start_with_request' => 'default', 'xdebug.start_upon_error' => 'yes' ] );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug02122.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> context_get -i 1 -d 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="1" context="0"><property name="$default" fullname="$default" type="string" size="13" encoding="base64"><![CDATA[ZGVmYXVsdCB2YWx1ZQ==]]></property><property name="$this" fullname="$this" type="object" classname="User" children="1" numchildren="1" page="0" pagesize="32"><property name="name" fullname="$this-&gt;name" facet="private" type="string" size="9" encoding="base64"><![CDATA[RWxlcGhwYW50]]></property></property></response>

-> context_get -i 2 -d 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="2" context="0"><property name="$e" fullname="$e" type="uninitialized"></property><property name="$u" fullname="$u" type="object" classname="User" children="1" numchildren="1" page="0" pagesize="32"><property name="name" fullname="$u-&gt;name" facet="private" type="string" size="9" encoding="base64"><![CDATA[RWxlcGhwYW50]]></property></property></response>

-> detach -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="3" status="stopping" reason="ok"></response>
