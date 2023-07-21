--TEST--
Test for bug #614: local variables loses class members (>= PHP 8.1)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1; dbgp');
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$filename = realpath( dirname(__FILE__) . '/bug00614.inc' );

$commands = array(
	"breakpoint_set -t line -f file://{$filename} -n 62",
	'run',
	'stack_get',
	'property_get -n $e',
	'property_get -n $e -p 1',
	'property_get -n $e->k',
	'property_get -n $e->*Base*k',
	'detach'
);

dbgpRunFile( $filename, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://bug00614.inc" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file://bug00614.inc -n 62
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id="{{PID}}0001"></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file://bug00614.inc" lineno="62"></xdebug:message></response>

-> stack_get -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="stack_get" transaction_id="3"><stack where="{main}" level="0" type="file" filename="file://bug00614.inc" lineno="62"></stack></response>

-> property_get -i 4 -n $e
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="$e" fullname="$e" type="object" classname="Extension" children="1" numchildren="33" page="0" pagesize="32"><property name="*Base*a" fullname="$e-&gt;*Base*a" facet="private" type="null"></property><property name="*Base*b" fullname="$e-&gt;*Base*b" facet="private" type="null"></property><property name="*Base*c" fullname="$e-&gt;*Base*c" facet="private" type="null"></property><property name="*Base*d" fullname="$e-&gt;*Base*d" facet="private" type="null"></property><property name="*Base*e" fullname="$e-&gt;*Base*e" facet="private" type="null"></property><property name="*Base*f" fullname="$e-&gt;*Base*f" facet="private" type="null"></property><property name="*Base*g" fullname="$e-&gt;*Base*g" facet="private" type="null"></property><property name="*Base*h" fullname="$e-&gt;*Base*h" facet="private" type="null"></property><property name="*Base*i" fullname="$e-&gt;*Base*i" facet="private" type="int"><![CDATA[99]]></property><property name="*Base*j" fullname="$e-&gt;*Base*j" facet="private" type="int"><![CDATA[88]]></property><property name="*Base*k" fullname="$e-&gt;*Base*k" facet="private" type="int"><![CDATA[11]]></property><property name="*Base*l" fullname="$e-&gt;*Base*l" facet="private" type="null"></property><property name="*Base*m" fullname="$e-&gt;*Base*m" facet="private" type="null"></property><property name="*Base*n" fullname="$e-&gt;*Base*n" facet="private" type="null"></property><property name="*Base*o" fullname="$e-&gt;*Base*o" facet="private" type="null"></property><property name="*Base*p" fullname="$e-&gt;*Base*p" facet="private" type="null"></property><property name="*Base*q" fullname="$e-&gt;*Base*q" facet="private" type="null"></property><property name="*Base*r" fullname="$e-&gt;*Base*r" facet="private" type="null"></property><property name="*Base*s" fullname="$e-&gt;*Base*s" facet="private" type="null"></property><property name="*Base*t" fullname="$e-&gt;*Base*t" facet="private" type="null"></property><property name="*Base*u" fullname="$e-&gt;*Base*u" facet="private" type="null"></property><property name="*Base*v" fullname="$e-&gt;*Base*v" facet="private" type="null"></property><property name="*Base*w" fullname="$e-&gt;*Base*w" facet="private" type="null"></property><property name="*Base*x" fullname="$e-&gt;*Base*x" facet="private" type="null"></property><property name="*Base*y" fullname="$e-&gt;*Base*y" facet="private" type="null"></property><property name="*Base*z" fullname="$e-&gt;*Base*z" facet="private" type="null"></property><property name="*Base*z1" fullname="$e-&gt;*Base*z1" facet="private" type="null"></property><property name="*Base*z2" fullname="$e-&gt;*Base*z2" facet="private" type="null"></property><property name="*Base*z3" fullname="$e-&gt;*Base*z3" facet="private" type="null"></property><property name="*Base*z4" fullname="$e-&gt;*Base*z4" facet="private" type="null"></property><property name="k" fullname="$e-&gt;k" facet="private" type="int"><![CDATA[77]]></property><property name="m" fullname="$e-&gt;m" facet="private" type="int"><![CDATA[66]]></property></property></response>

-> property_get -i 5 -n $e -p 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$e" fullname="$e" type="object" classname="Extension" children="1" numchildren="33" page="1" pagesize="32"><property name="j" fullname="$e-&gt;j" facet="private" type="int"><![CDATA[55]]></property></property></response>

-> property_get -i 6 -n $e->k
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="6"><property name="$e-&gt;k" fullname="$e-&gt;k" type="int"><![CDATA[77]]></property></response>

-> property_get -i 7 -n $e->*Base*k
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="7"><property name="$e-&gt;*Base*k" fullname="$e-&gt;*Base*k" type="int"><![CDATA[11]]></property></response>

-> detach -i 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" command="detach" transaction_id="8" status="stopping" reason="ok"></response>
