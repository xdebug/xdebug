--TEST--
Test for bug #1048: Can not get $GLOBAL variable by property_value on function context
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';

$dir = dirname(__FILE__);
putenv("XDEBUG_TEST_DIR=$dir");

$data = file_get_contents(dirname(__FILE__) . '/bug01048.inc');

$commands = array(
	'run',
	'property_get -d 0 -c 1 -n $GLOBALS[\'cache\']',
	'property_value -d 0 -c 1 -n $GLOBALS[\'cache\']',
	'run',
	'property_get -d 0 -c 1 -n $GLOBALS[\'cache\']',
	'property_value -d 0 -c 1 -n $GLOBALS[\'cache\']',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///%sxdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-%d by Derick Rethans]]></copyright></init>

-> run -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="6"></xdebug:message></response>

-> property_get -i 2 -d 0 -c 1 -n $GLOBALS['cache']
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="2"><property name="$GLOBALS[&#39;cache&#39;]" fullname="$GLOBALS[&#39;cache&#39;]" type="string" size="5" encoding="base64"><![CDATA[Y2FjaGU=]]></property></response>

-> property_value -i 3 -d 0 -c 1 -n $GLOBALS['cache']
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_value" transaction_id="3" type="string" size="5" encoding="base64"><![CDATA[Y2FjaGU=]]></response>

-> run -i 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="4" status="break" reason="ok"><xdebug:message filename="file:///%sxdebug-dbgp-test.php" lineno="11"></xdebug:message></response>

-> property_get -i 5 -d 0 -c 1 -n $GLOBALS['cache']
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="$GLOBALS[&#39;cache&#39;]" fullname="$GLOBALS[&#39;cache&#39;]" type="string" size="5" encoding="base64"><![CDATA[Y2FjaGU=]]></property></response>

-> property_value -i 6 -d 0 -c 1 -n $GLOBALS['cache']
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_value" transaction_id="6" type="string" size="5" encoding="base64"><![CDATA[Y2FjaGU=]]></response>
