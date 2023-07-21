--TEST--
xdebug_notify()
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/xdebug_notify.inc';

$commands = array(
	'feature_set -n notify_ok -v 1',
	'feature_set -n extended_properties -v 1',
	'run',
	'detach',
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://xdebug_notify.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n notify_ok -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="notify_ok" success="1"></response>

-> feature_set -i 2 -n extended_properties -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="extended_properties" success="1"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<notify xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" name="user"><xdebug:location filename="file://xdebug_notify.inc" lineno="2"></xdebug:location><property type="array" children="1" numchildren="24" page="0" pagesize="32"><property name="0" type="string" size="1" encoding="base64"><![CDATA[YQ==]]></property><property name="1" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property><property name="2" type="string" size="1" encoding="base64"><![CDATA[Yw==]]></property><property name="3" type="string" size="1" encoding="base64"><![CDATA[ZA==]]></property><property name="4" type="string" size="1" encoding="base64"><![CDATA[ZQ==]]></property><property name="5" type="string" size="1" encoding="base64"><![CDATA[Zg==]]></property><property name="6" type="string" size="1" encoding="base64"><![CDATA[Zw==]]></property><property name="7" type="string" size="1" encoding="base64"><![CDATA[aA==]]></property><property name="8" type="string" size="1" encoding="base64"><![CDATA[aQ==]]></property><property name="9" type="string" size="1" encoding="base64"><![CDATA[ag==]]></property><property name="10" type="string" size="1" encoding="base64"><![CDATA[aw==]]></property><property name="11" type="string" size="1" encoding="base64"><![CDATA[bA==]]></property><property name="12" type="string" size="1" encoding="base64"><![CDATA[bQ==]]></property><property name="13" type="string" size="1" encoding="base64"><![CDATA[bg==]]></property><property name="14" type="string" size="1" encoding="base64"><![CDATA[bw==]]></property><property name="15" type="string" size="1" encoding="base64"><![CDATA[cA==]]></property><property name="16" type="string" size="1" encoding="base64"><![CDATA[cQ==]]></property><property name="17" type="string" size="1" encoding="base64"><![CDATA[cg==]]></property><property name="18" type="string" size="1" encoding="base64"><![CDATA[cw==]]></property><property name="19" type="string" size="1" encoding="base64"><![CDATA[dA==]]></property><property name="20" type="string" size="1" encoding="base64"><![CDATA[dQ==]]></property><property name="21" type="string" size="1" encoding="base64"><![CDATA[dg==]]></property><property name="22" type="string" size="1" encoding="base64"><![CDATA[dw==]]></property><property name="23" type="string" size="1" encoding="base64"><![CDATA[eA==]]></property></property></notify>

<?xml version="1.0" encoding="iso-8859-1"?>
<notify xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" name="user"><xdebug:location filename="file://xdebug_notify.inc" lineno="3"></xdebug:location><property type="array" children="1" numchildren="1" page="0" pagesize="32"><property type="int"><name encoding="base64"><![CDATA[dGVzdABiYXI=]]></name><value encoding="base64"><![CDATA[NDI=]]></value></property></property></notify>

<?xml version="1.0" encoding="iso-8859-1"?>
<notify xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" name="user"><xdebug:location filename="file://xdebug_notify.inc" lineno="4"></xdebug:location><property type="array" children="1" numchildren="1" page="0" pagesize="32"><property type="string" size="7"><name encoding="base64"><![CDATA[dGVzdA==]]></name><value encoding="base64"><![CDATA[Zm9vAGJhcg==]]></value></property></property></notify>

<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="stopping" reason="ok"></response>

-> detach -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="4" status="stopping" reason="ok"></response>
