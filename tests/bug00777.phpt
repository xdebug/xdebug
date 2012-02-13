--TEST--
Test for bug #777: Connection to reset on stepping over call to mysqli_init
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = file_get_contents(dirname(__FILE__) . '/bug00777.inc');

$commands = array(
	'breakpoint_set -t line -f file:///tmp/xdebug-dbgp-test.php -n 5',
	'run',
	'step_over',
	'context_get',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///tmp/xdebug-dbgp-test.php" language="PHP" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2012 by Derick Rethans]]></copyright></init>

-> breakpoint_set -i 1 -t line -f file:///tmp/xdebug-dbgp-test.php -n 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="1" id=""></response>

-> run -i 2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="2" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="5"></xdebug:message></response>

-> step_over -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_over" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="6"></xdebug:message></response>

-> context_get -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"><property name="$connection" fullname="$connection" address="" type="object" classname="mysqli" children="1" numchildren="18" page="0" pagesize="32"><property name="affected_rows" fullname="$connection-&gt;affected_rows" facet="public" address="" type="null"></property><property name="client_info" fullname="$connection-&gt;client_info" facet="public" address="" type="null"></property><property name="client_version" fullname="$connection-&gt;client_version" facet="public" address="" type="null"></property><property name="connect_errno" fullname="$connection-&gt;connect_errno" facet="public" address="" type="null"></property><property name="connect_error" fullname="$connection-&gt;connect_error" facet="public" address="" type="null"></property><property name="errno" fullname="$connection-&gt;errno" facet="public" address="" type="null"></property><property name="error" fullname="$connection-&gt;error" facet="public" address="" type="null"></property><property name="field_count" fullname="$connection-&gt;field_count" facet="public" address="" type="null"></property><property name="host_info" fullname="$connection-&gt;host_info" facet="public" address="" type="null"></property><property name="info" fullname="$connection-&gt;info" facet="public" address="" type="null"></property><property name="insert_id" fullname="$connection-&gt;insert_id" facet="public" address="" type="null"></property><property name="server_info" fullname="$connection-&gt;server_info" facet="public" address="" type="null"></property><property name="server_version" fullname="$connection-&gt;server_version" facet="public" address="" type="null"></property><property name="stat" fullname="$connection-&gt;stat" facet="public" address="" type="null"></property><property name="sqlstate" fullname="$connection-&gt;sqlstate" facet="public" address="" type="null"></property><property name="protocol_version" fullname="$connection-&gt;protocol_version" facet="public" address="" type="null"></property><property name="thread_id" fullname="$connection-&gt;thread_id" facet="public" address="" type="null"></property><property name="warning_count" fullname="$connection-&gt;warning_count" facet="public" address="" type="null"></property></property></response>
