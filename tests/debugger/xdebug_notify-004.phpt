--TEST--
xdebug_notify() with custom settings
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/xdebug_notify_complex.inc';

$commands = array(
	'feature_set -n notify_ok -v 1',
	'feature_set -n extended_properties -v 1',
	'run',
	'detach',
);

dbgpRunFile( $filename, $commands, [ 'xdebug.var_display_max_depth' => 1, 'xdebug.var_display_max_children' => 4, 'xdebug.var_display_max_data' => 4 ] );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://xdebug_notify_complex.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> feature_set -i 1 -n notify_ok -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="1" feature="notify_ok" success="1"></response>

-> feature_set -i 2 -n extended_properties -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="extended_properties" success="1"></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<notify xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" name="user"><xdebug:location filename="file://xdebug_notify_complex.inc" lineno="20"></xdebug:location><property type="array" children="1" numchildren="3" page="0" pagesize="4"><property name="0" type="array" children="1" numchildren="1"></property><property name="1" type="array" children="1" numchildren="1"></property><property name="2" type="array" children="1" numchildren="1"></property></property></notify>

<?xml version="1.0" encoding="iso-8859-1"?>
<notify xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" name="user"><xdebug:location filename="file://xdebug_notify_complex.inc" lineno="20"></xdebug:location><property type="array" children="1" numchildren="3" page="0" pagesize="2"><property name="0" type="array" children="1" numchildren="1" page="0" pagesize="2"><property name="two" fullname="[&quot;two&quot;]" type="array" children="1" numchildren="1"></property></property><property name="1" type="array" children="1" numchildren="1" page="0" pagesize="2"><property name="long_string" fullname="[&quot;long_string&quot;]" type="string" size="1024" encoding="base64"><![CDATA[YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWE=]]></property></property></property></notify>

<?xml version="1.0" encoding="iso-8859-1"?>
<notify xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" name="user"><xdebug:location filename="file://xdebug_notify_complex.inc" lineno="20"></xdebug:location><property type="array" children="1" numchildren="3" page="0" pagesize="2"><property name="0" type="array" children="1" numchildren="1" page="0" pagesize="2"><property name="two" fullname="[&quot;two&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="2"><property name="trois" fullname="[&quot;two&quot;][&quot;trois&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="2"><property name="0" fullname="[&quot;two&quot;][&quot;trois&quot;][0]" type="string" size="3" encoding="base64"><![CDATA[YmFy]]></property></property></property></property><property name="1" type="array" children="1" numchildren="1" page="0" pagesize="2"><property name="long_string" fullname="[&quot;long_string&quot;]" type="string" size="1024" encoding="base64"><![CDATA[YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWE=]]></property></property></property></notify>

<?xml version="1.0" encoding="iso-8859-1"?>
<notify xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" name="user"><xdebug:location filename="file://xdebug_notify_complex.inc" lineno="20"></xdebug:location><property type="array" children="1" numchildren="3" page="0" pagesize="16"><property name="0" type="array" children="1" numchildren="1" page="0" pagesize="16"><property name="two" fullname="[&quot;two&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="16"><property name="trois" fullname="[&quot;two&quot;][&quot;trois&quot;]" type="array" children="1" numchildren="1" page="0" pagesize="16"><property name="0" fullname="[&quot;two&quot;][&quot;trois&quot;][0]" type="string" size="3" encoding="base64"><![CDATA[YmFy]]></property></property></property></property><property name="1" type="array" children="1" numchildren="1" page="0" pagesize="16"><property name="long_string" fullname="[&quot;long_string&quot;]" type="string" size="1024" encoding="base64"><![CDATA[YWFhYQ==]]></property></property><property name="2" type="array" children="1" numchildren="1" page="0" pagesize="16"><property name="letters" fullname="[&quot;letters&quot;]" type="array" children="1" numchildren="24" page="0" pagesize="16"><property name="0" fullname="[&quot;letters&quot;][0]" type="string" size="1" encoding="base64"><![CDATA[YQ==]]></property><property name="1" fullname="[&quot;letters&quot;][1]" type="string" size="1" encoding="base64"><![CDATA[Yg==]]></property><property name="2" fullname="[&quot;letters&quot;][2]" type="string" size="1" encoding="base64"><![CDATA[Yw==]]></property><property name="3" fullname="[&quot;letters&quot;][3]" type="string" size="1" encoding="base64"><![CDATA[ZA==]]></property><property name="4" fullname="[&quot;letters&quot;][4]" type="string" size="1" encoding="base64"><![CDATA[ZQ==]]></property><property name="5" fullname="[&quot;letters&quot;][5]" type="string" size="1" encoding="base64"><![CDATA[Zg==]]></property><property name="6" fullname="[&quot;letters&quot;][6]" type="string" size="1" encoding="base64"><![CDATA[Zw==]]></property><property name="7" fullname="[&quot;letters&quot;][7]" type="string" size="1" encoding="base64"><![CDATA[aA==]]></property><property name="8" fullname="[&quot;letters&quot;][8]" type="string" size="1" encoding="base64"><![CDATA[aQ==]]></property><property name="9" fullname="[&quot;letters&quot;][9]" type="string" size="1" encoding="base64"><![CDATA[ag==]]></property><property name="10" fullname="[&quot;letters&quot;][10]" type="string" size="1" encoding="base64"><![CDATA[aw==]]></property><property name="11" fullname="[&quot;letters&quot;][11]" type="string" size="1" encoding="base64"><![CDATA[bA==]]></property><property name="12" fullname="[&quot;letters&quot;][12]" type="string" size="1" encoding="base64"><![CDATA[bQ==]]></property><property name="13" fullname="[&quot;letters&quot;][13]" type="string" size="1" encoding="base64"><![CDATA[bg==]]></property><property name="14" fullname="[&quot;letters&quot;][14]" type="string" size="1" encoding="base64"><![CDATA[bw==]]></property><property name="15" fullname="[&quot;letters&quot;][15]" type="string" size="1" encoding="base64"><![CDATA[cA==]]></property></property></property></property></notify>

<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="stopping" reason="ok"></response>

-> detach -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="4" status="stopping" reason="ok"></response>
