--TEST--
Test for bug #421: xdebug sends back invalid characters in xml sometimes
--SKIPIF--
<?php
if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); }
if (in_array('SimpleXMLIterator', get_declared_classes()) == false) { echo "skip SimpleXML extension required\n"; }
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$data = file_get_contents( dirname(__FILE__) . '/bug00421.inc' );

$commands = array(
	'step_into',
	'feature_set -n max_depth -v 2',
	'breakpoint_set -t line -n 25',
	'run',
	'context_get -c 0',
//	'property_get -n currentPageXML',
	'detach'
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///%sxdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2%d by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="2"></xdebug:message></response>

-> feature_set -i 2 -n max_depth -v 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="feature_set" transaction_id="2" feature="max_depth" success="1"></response>

-> breakpoint_set -i 3 -t line -n 25
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="3" id=""></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="25"></xdebug:message></response>

-> context_get -i 5 -c 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="5" context="0"><property name="$currentPageXML" fullname="$currentPageXML" type="object" classname="SimpleXMLIterator" children="1" numchildren="2" page="0" pagesize="32"><property name="@attributes" fullname="$currentPageXML-&gt;@attributes" facet="public" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="name" fullname="$currentPageXML-&gt;@attributes[&#39;name&#39;]" type="string" size="8" encoding="base64"><![CDATA[cHJvamVjdHM=]]></property></property><property name="page" fullname="$currentPageXML-&gt;page" facet="public" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="0" fullname="$currentPageXML-&gt;page[0]" type="object" classname="SimpleXMLIterator" children="1" numchildren="1"></property><property name="1" fullname="$currentPageXML-&gt;page[1]" type="object" classname="SimpleXMLIterator" children="1" numchildren="1"></property></property></property><property name="$iterator" fullname="$iterator" type="object" classname="SimpleXMLIterator" children="1" numchildren="1" page="0" pagesize="32"><property name="page" fullname="$iterator-&gt;page" facet="public" type="object" classname="SimpleXMLIterator" children="1" numchildren="2" page="0" pagesize="32"><property name="@attributes" fullname="$iterator-&gt;page-&gt;@attributes" facet="public" type="array" children="1" numchildren="1"></property><property name="page" fullname="$iterator-&gt;page-&gt;page" facet="public" type="array" children="1" numchildren="2"></property></property></property><property name="$name" fullname="$name" type="uninitialized"></property><property name="$pageXML" fullname="$pageXML" type="object" classname="SimpleXMLIterator" children="1" numchildren="2" page="0" pagesize="32"><property name="@attributes" fullname="$pageXML-&gt;@attributes" facet="public" type="array" children="1" numchildren="1" page="0" pagesize="32"><property name="name" fullname="$pageXML-&gt;@attributes[&#39;name&#39;]" type="string" size="8" encoding="base64"><![CDATA[cHJvamVjdHM=]]></property></property><property name="page" fullname="$pageXML-&gt;page" facet="public" type="array" children="1" numchildren="2" page="0" pagesize="32"><property name="0" fullname="$pageXML-&gt;page[0]" type="object" classname="SimpleXMLIterator" children="1" numchildren="1"></property><property name="1" fullname="$pageXML-&gt;page[1]" type="object" classname="SimpleXMLIterator" children="1" numchildren="1"></property></property></property><property name="$projectsIterator" fullname="$projectsIterator" type="array" children="1" numchildren="3" page="0" pagesize="32"><property name="0" fullname="$projectsIterator[0]" type="object" classname="SimpleXMLIterator" children="1" numchildren="2" page="0" pagesize="32"><property name="@attributes" fullname="$projectsIterator[0]-&gt;@attributes" facet="public" type="array" children="1" numchildren="1"></property><property name="page" fullname="$projectsIterator[0]-&gt;page" facet="public" type="array" children="1" numchildren="2"></property></property><property name="1" fullname="$projectsIterator[1]" type="object" classname="SimpleXMLIterator" children="1" numchildren="1" page="0" pagesize="32"><property name="@attributes" fullname="$projectsIterator[1]-&gt;@attributes" facet="public" type="array" children="1" numchildren="1"></property></property><property name="2" fullname="$projectsIterator[2]" type="object" classname="SimpleXMLIterator" children="1" numchildren="1" page="0" pagesize="32"><property name="@attributes" fullname="$projectsIterator[2]-&gt;@attributes" facet="public" type="array" children="1" numchildren="1"></property></property></property><property name="$siteXMLString" fullname="$siteXMLString" type="string" size="161" encoding="base64"><![CDATA[PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iVVRGLTgiPz4KPHNpdGU+CiAgICA8cGFnZSBuYW1lPSJwcm9qZWN0cyI+CiAgICAgICAgPHBhZ2UgbmFtZT0iUHJvamVjdCAxIiAvPgogICAgICAgIDxwYWdlIG5hbWU9IlByb2plY3QgMiIgLz4KICAgIDwvcGFnZT4KPC9zaXRlPgo=]]></property></response>

-> detach -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="6" status="stopping" reason="ok"></response>
