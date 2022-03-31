--TEST--
Test for bug #1488: Rewrite DBGp 'property_set' to always use eval
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug01488.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 4",
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

dbgpRunFile( $filename, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug01488.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug01488.inc -n 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="{{PID}}0001"></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug01488.inc" lineno="4"></xdebug:message></response>

-> property_set -i 3 -n $var -t string -- InNjb3BlMC1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="3" success="1"></response>

-> property_get -i 4 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$var" fullname="$var" type="string" size="15" encoding="base64"><![CDATA[c2NvcGUwLW1vZGlmaWVk]]></property></response>

-> property_set -i 5 -n $var -t string -- dHJ1ZQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="5" success="1"></response>

-> property_get -i 6 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$var" fullname="$var" type="string" size="1" encoding="base64"><![CDATA[MQ==]]></property></response>

-> property_set -i 7 -n $var -t string -- ZmFsc2U=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="7" success="1"></response>

-> property_get -i 8 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="8"><property name="$var" fullname="$var" type="string" size="0" encoding="base64"><![CDATA[]]></property></response>

-> property_set -i 9 -n $var -t string -- NDI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="9" success="1"></response>

-> property_get -i 10 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="10"><property name="$var" fullname="$var" type="string" size="2" encoding="base64"><![CDATA[NDI=]]></property></response>

-> property_set -i 11 -n $var -t string -- IjQyIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="11" success="1"></response>

-> property_get -i 12 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="12"><property name="$var" fullname="$var" type="string" size="2" encoding="base64"><![CDATA[NDI=]]></property></response>

-> property_set -i 13 -n $var -t string -- NDIuMTE=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="13" success="1"></response>

-> property_get -i 14 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="14"><property name="$var" fullname="$var" type="string" size="5" encoding="base64"><![CDATA[NDIuMTE=]]></property></response>

-> property_set -i 15 -n $var -t string -- IjQyLjExIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="15" success="1"></response>

-> property_get -i 16 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="16"><property name="$var" fullname="$var" type="string" size="5" encoding="base64"><![CDATA[NDIuMTE=]]></property></response>

-> property_set -i 17 -n $var -t string -- MHg1RQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="17" success="1"></response>

-> property_get -i 18 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="18"><property name="$var" fullname="$var" type="string" size="2" encoding="base64"><![CDATA[OTQ=]]></property></response>

-> property_set -i 19 -n $var -t bool -- InNjb3BlMC1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="19" success="1"></response>

-> property_get -i 20 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="20"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 21 -n $var -t bool -- dHJ1ZQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="21" success="1"></response>

-> property_get -i 22 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="22"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 23 -n $var -t bool -- ZmFsc2U=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="23" success="1"></response>

-> property_get -i 24 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="24"><property name="$var" fullname="$var" type="bool"><![CDATA[0]]></property></response>

-> property_set -i 25 -n $var -t bool -- NDI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="25" success="1"></response>

-> property_get -i 26 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="26"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 27 -n $var -t bool -- IjQyIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="27" success="1"></response>

-> property_get -i 28 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="28"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 29 -n $var -t bool -- NDIuMTE=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="29" success="1"></response>

-> property_get -i 30 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="30"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 31 -n $var -t bool -- IjQyLjExIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="31" success="1"></response>

-> property_get -i 32 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="32"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 33 -n $var -t bool -- MHg1RQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="33" success="1"></response>

-> property_get -i 34 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="34"><property name="$var" fullname="$var" type="bool"><![CDATA[1]]></property></response>

-> property_set -i 35 -n $var -t int -- InNjb3BlMC1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="35" success="1"></response>

-> property_get -i 36 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="36"><property name="$var" fullname="$var" type="int"><![CDATA[0]]></property></response>

-> property_set -i 37 -n $var -t int -- dHJ1ZQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="37" success="1"></response>

-> property_get -i 38 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="38"><property name="$var" fullname="$var" type="int"><![CDATA[1]]></property></response>

-> property_set -i 39 -n $var -t int -- ZmFsc2U=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="39" success="1"></response>

-> property_get -i 40 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="40"><property name="$var" fullname="$var" type="int"><![CDATA[0]]></property></response>

-> property_set -i 41 -n $var -t int -- NDI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="41" success="1"></response>

-> property_get -i 42 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="42"><property name="$var" fullname="$var" type="int"><![CDATA[42]]></property></response>

-> property_set -i 43 -n $var -t int -- IjQyIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="43" success="1"></response>

-> property_get -i 44 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="44"><property name="$var" fullname="$var" type="int"><![CDATA[42]]></property></response>

-> property_set -i 45 -n $var -t int -- NDIuMTE=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="45" success="1"></response>

-> property_get -i 46 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="46"><property name="$var" fullname="$var" type="int"><![CDATA[42]]></property></response>

-> property_set -i 47 -n $var -t int -- IjQyLjExIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="47" success="1"></response>

-> property_get -i 48 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="48"><property name="$var" fullname="$var" type="int"><![CDATA[42]]></property></response>

-> property_set -i 49 -n $var -t int -- MHg1RQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="49" success="1"></response>

-> property_get -i 50 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="50"><property name="$var" fullname="$var" type="int"><![CDATA[94]]></property></response>

-> property_set -i 51 -n $var -t float -- InNjb3BlMC1tb2RpZmllZCI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="51" success="1"></response>

-> property_get -i 52 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="52"><property name="$var" fullname="$var" type="float"><![CDATA[0]]></property></response>

-> property_set -i 53 -n $var -t float -- dHJ1ZQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="53" success="1"></response>

-> property_get -i 54 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="54"><property name="$var" fullname="$var" type="float"><![CDATA[1]]></property></response>

-> property_set -i 55 -n $var -t float -- ZmFsc2U=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="55" success="1"></response>

-> property_get -i 56 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="56"><property name="$var" fullname="$var" type="float"><![CDATA[0]]></property></response>

-> property_set -i 57 -n $var -t float -- NDI=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="57" success="1"></response>

-> property_get -i 58 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="58"><property name="$var" fullname="$var" type="float"><![CDATA[42]]></property></response>

-> property_set -i 59 -n $var -t float -- IjQyIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="59" success="1"></response>

-> property_get -i 60 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="60"><property name="$var" fullname="$var" type="float"><![CDATA[42]]></property></response>

-> property_set -i 61 -n $var -t float -- NDIuMTE=
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="61" success="1"></response>

-> property_get -i 62 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="62"><property name="$var" fullname="$var" type="float"><![CDATA[42.11]]></property></response>

-> property_set -i 63 -n $var -t float -- IjQyLjExIg==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="63" success="1"></response>

-> property_get -i 64 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="64"><property name="$var" fullname="$var" type="float"><![CDATA[42.11]]></property></response>

-> property_set -i 65 -n $var -t float -- MHg1RQ==
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_set" transaction_id="65" success="1"></response>

-> property_get -i 66 -n $var
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="66"><property name="$var" fullname="$var" type="float"><![CDATA[94]]></property></response>

-> detach -i 67
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="67" status="stopping" reason="ok"></response>
