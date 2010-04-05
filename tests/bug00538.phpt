--TEST--
Test for bug #538: Error in watches and call stack parameter with string containing '\\'
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = <<<'NOWDOC'
<?php
    function call($param1, $param2, $param3)
    {
        echo $param1;
        echo $param2;
        echo $param3;

		var_dump(xdebug_get_function_stack());
    }

    $test=getcwd();
	$value = 'candena\\a\nb';
	echo $value, "\n";
    call($test, $value, 'caneda \\\a \\\\b \\\\\c|');
NOWDOC;

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 8',
	'run',
	'property_get -n param2',
	'property_get -n param3',
	'detach'
);

dbgpRun( $data, $commands );
?>
--EXPECT--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///tmp/xdebug-dbgp-test.php" language="PHP" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2010 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 8
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="8"></xdebug:message></response>

-> property_get -i 4 -n param2
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="param2" fullname="$param2" address="" type="string" size="12" encoding="base64"><![CDATA[Y2FuZGVuYVxhXG5i]]></property></response>

-> property_get -i 5 -n param3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="5"><property name="param3" fullname="$param3" address="" type="string" size="20" encoding="base64"><![CDATA[Y2FuZWRhIFxcYSBcXGIgXFxcY3w=]]></property></response>

-> detach -i 6
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="6" status="stopping" reason="ok"></response>
