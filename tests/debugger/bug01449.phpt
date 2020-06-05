--TEST--
Test for bug #1449: Array element keys containing low-ASCII variables
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$filename = dirname(__FILE__) . '/bug01449.inc';

$commands = array(
	'step_into',
	'feature_set -n extended_properties -v 1',
	'breakpoint_set -t line -n 7',
	'run',
	'property_get -n $var',
	'detach'
);

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01449.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file://bug01449.inc" lineno="2"></xdebug:message></response>

-> feature_set -i 2 -n extended_properties -v 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="extended_properties" success="1"></response>

-> breakpoint_set -i 3 -t line -n 7
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id=""></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file://bug01449.inc" lineno="7"></xdebug:message></response>

-> property_get -i 5 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$var" fullname="$var" type="array" children="1" numchildren="32" page="0" pagesize="32"><property type="string" size="1"><name encoding="base64"><![CDATA[MC0ALTA=]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMC1cMC0wIl0=]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MS0BLTE=]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMS0BLTEiXQ==]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[Mi0CLTI=]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMi0CLTIiXQ==]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[My0DLTM=]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMy0DLTMiXQ==]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[NC0ELTQ=]]></name><fullname encoding="base64"><![CDATA[JHZhclsiNC0ELTQiXQ==]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[NS0FLTU=]]></name><fullname encoding="base64"><![CDATA[JHZhclsiNS0FLTUiXQ==]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[Ni0GLTY=]]></name><fullname encoding="base64"><![CDATA[JHZhclsiNi0GLTYiXQ==]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[Ny0HLTc=]]></name><fullname encoding="base64"><![CDATA[JHZhclsiNy0HLTciXQ==]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[OC0ILTg=]]></name><fullname encoding="base64"><![CDATA[JHZhclsiOC0ILTgiXQ==]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[OS0JLTk=]]></name><fullname encoding="base64"><![CDATA[JHZhclsiOS0JLTkiXQ==]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MTAtCi0xMA==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMTAtCi0xMCJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MTEtCy0xMQ==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMTEtCy0xMSJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MTItDC0xMg==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMTItDC0xMiJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MTMtDS0xMw==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMTMtDS0xMyJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MTQtDi0xNA==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMTQtDi0xNCJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MTUtDy0xNQ==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMTUtDy0xNSJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MTYtEC0xNg==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMTYtEC0xNiJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MTctES0xNw==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMTctES0xNyJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MTgtEi0xOA==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMTgtEi0xOCJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MTktEy0xOQ==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMTktEy0xOSJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MjAtFC0yMA==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMjAtFC0yMCJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MjEtFS0yMQ==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMjEtFS0yMSJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MjItFi0yMg==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMjItFi0yMiJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MjMtFy0yMw==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMjMtFy0yMyJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MjQtGC0yNA==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMjQtGC0yNCJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MjUtGS0yNQ==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMjUtGS0yNSJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MjYtGi0yNg==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMjYtGi0yNiJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MjctGy0yNw==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMjctGy0yNyJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MjgtHC0yOA==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMjgtHC0yOCJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MjktHS0yOQ==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMjktHS0yOSJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MzAtHi0zMA==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMzAtHi0zMCJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property><property type="string" size="1"><name encoding="base64"><![CDATA[MzEtHy0zMQ==]]></name><fullname encoding="base64"><![CDATA[JHZhclsiMzEtHy0zMSJd]]></fullname><value encoding="base64"><![CDATA[dg==]]></value></property></property></response>

-> detach -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="6" status="stopping" reason="ok"></response>
