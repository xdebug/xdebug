--TEST--
Test for bug #1488: Rewrite DBGp 'property_set' to always use eval
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = dirname(__FILE__) . '/bug01488.inc';

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 4',
	'run',
);

$types = [ 'string', 'bool', 'int', 'float' ];
$values = [ '"scope0-modified"', 'true', 'false', '42', '"42"', '42.11', '"42.11"', "0x5E" ];

foreach ( $types as $type )
{
	foreach ( $values as $value )
	{
		$commands[] = 'property_set -n $var -t ' . $type . ' -- ' . base64_encode( $value );
		$commands[] = 'property_get -n $var';
	}
}

$commands[] = 'detach';

dbgpRun( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file:///%s" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%s" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///%s" lineno="4"></xdebug:message></response>

-> property_set -i 4 -n $var -t string -- InNjb3BlMC1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="4" success="1"></response>

-> property_get -i 5 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$var" fullname="$var" type="string" size="15" encoding="base64"><![CDATA[c2NvcGUwLW1vZGlmaWVk]]></property></response>

-> property_set -i 6 -n $var -t string -- dHJ1ZQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="6" success="1"></response>

-> property_get -i 7 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$var" fullname="$var" type="string" size="1" encoding="base64"><![CDATA[MQ==]]></property></response>

-> property_set -i 8 -n $var -t string -- ZmFsc2U=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="8" success="1"></response>

-> property_get -i 9 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="9"><property name="$var" fullname="$var" type="string" size="0" encoding="base64"><![CDATA[]]></property></response>

-> property_set -i 10 -n $var -t string -- NDI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="10" success="1"></response>

-> property_get -i 11 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="11"><property name="$var" fullname="$var" type="string" size="2" encoding="base64"><![CDATA[NDI=]]></property></response>

-> property_set -i 12 -n $var -t string -- IjQyIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="12" success="1"></response>

-> property_get -i 13 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="13"><property name="$var" fullname="$var" type="string" size="2" encoding="base64"><![CDATA[NDI=]]></property></response>

-> property_set -i 14 -n $var -t string -- NDIuMTE=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="14" success="1"></response>

-> property_get -i 15 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="15"><property name="$var" fullname="$var" type="string" size="5" encoding="base64"><![CDATA[NDIuMTE=]]></property></response>

-> property_set -i 16 -n $var -t string -- IjQyLjExIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="16" success="1"></response>

-> property_get -i 17 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="17"><property name="$var" fullname="$var" type="string" size="5" encoding="base64"><![CDATA[NDIuMTE=]]></property></response>

-> property_set -i 18 -n $var -t string -- MHg1RQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="18" success="1"></response>

-> property_get -i 19 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="19"><property name="$var" fullname="$var" type="string" size="2" encoding="base64"><![CDATA[OTQ=]]></property></response>

-> property_set -i 20 -n $var -t bool -- InNjb3BlMC1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="20" success="1"></response>

-> property_get -i 21 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="21"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 22 -n $var -t bool -- dHJ1ZQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="22" success="1"></response>

-> property_get -i 23 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="23"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 24 -n $var -t bool -- ZmFsc2U=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="24" success="1"></response>

-> property_get -i 25 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="25"><property name="$var" fullname="$var" type="bool"><![CDATA[0]]></property></response>

-> property_set -i 26 -n $var -t bool -- NDI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="26" success="1"></response>

-> property_get -i 27 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="27"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 28 -n $var -t bool -- IjQyIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="28" success="1"></response>

-> property_get -i 29 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="29"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 30 -n $var -t bool -- NDIuMTE=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="30" success="1"></response>

-> property_get -i 31 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="31"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 32 -n $var -t bool -- IjQyLjExIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="32" success="1"></response>

-> property_get -i 33 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="33"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 34 -n $var -t bool -- MHg1RQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="34" success="1"></response>

-> property_get -i 35 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="35"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 36 -n $var -t int -- InNjb3BlMC1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="36" success="1"></response>

-> property_get -i 37 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="37"><property name="$var" fullname="$var" type="int"><![CDATA[0]]></property></response>

-> property_set -i 38 -n $var -t int -- dHJ1ZQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="38" success="1"></response>

-> property_get -i 39 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="39"><property name="$var" fullname="$var" type="int"><![CDATA[1]]></property></response>

-> property_set -i 40 -n $var -t int -- ZmFsc2U=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="40" success="1"></response>

-> property_get -i 41 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="41"><property name="$var" fullname="$var" type="int"><![CDATA[0]]></property></response>

-> property_set -i 42 -n $var -t int -- NDI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="42" success="1"></response>

-> property_get -i 43 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="43"><property name="$var" fullname="$var" type="int"><![CDATA[42]]></property></response>

-> property_set -i 44 -n $var -t int -- IjQyIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="44" success="1"></response>

-> property_get -i 45 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="45"><property name="$var" fullname="$var" type="int"><![CDATA[42]]></property></response>

-> property_set -i 46 -n $var -t int -- NDIuMTE=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="46" success="1"></response>

-> property_get -i 47 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="47"><property name="$var" fullname="$var" type="int"><![CDATA[42]]></property></response>

-> property_set -i 48 -n $var -t int -- IjQyLjExIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="48" success="1"></response>

-> property_get -i 49 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="49"><property name="$var" fullname="$var" type="int"><![CDATA[42]]></property></response>

-> property_set -i 50 -n $var -t int -- MHg1RQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="50" success="1"></response>

-> property_get -i 51 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="51"><property name="$var" fullname="$var" type="int"><![CDATA[94]]></property></response>

-> property_set -i 52 -n $var -t float -- InNjb3BlMC1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="52" success="1"></response>

-> property_get -i 53 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="53"><property name="$var" fullname="$var" type="float"><![CDATA[0]]></property></response>

-> property_set -i 54 -n $var -t float -- dHJ1ZQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="54" success="1"></response>

-> property_get -i 55 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="55"><property name="$var" fullname="$var" type="float"><![CDATA[1]]></property></response>

-> property_set -i 56 -n $var -t float -- ZmFsc2U=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="56" success="1"></response>

-> property_get -i 57 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="57"><property name="$var" fullname="$var" type="float"><![CDATA[0]]></property></response>

-> property_set -i 58 -n $var -t float -- NDI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="58" success="1"></response>

-> property_get -i 59 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="59"><property name="$var" fullname="$var" type="float"><![CDATA[42]]></property></response>

-> property_set -i 60 -n $var -t float -- IjQyIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="60" success="1"></response>

-> property_get -i 61 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="61"><property name="$var" fullname="$var" type="float"><![CDATA[42]]></property></response>

-> property_set -i 62 -n $var -t float -- NDIuMTE=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="62" success="1"></response>

-> property_get -i 63 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="63"><property name="$var" fullname="$var" type="float"><![CDATA[42.11]]></property></response>

-> property_set -i 64 -n $var -t float -- IjQyLjExIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="64" success="1"></response>

-> property_get -i 65 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="65"><property name="$var" fullname="$var" type="float"><![CDATA[42.11]]></property></response>

-> property_set -i 66 -n $var -t float -- MHg1RQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="66" success="1"></response>

-> property_get -i 67 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="67"><property name="$var" fullname="$var" type="float"><![CDATA[94]]></property></response>

-> detach -i 68
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="68" status="stopping" reason="ok"></response>
