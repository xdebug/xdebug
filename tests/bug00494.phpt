--TEST--
Test for bug #494: Private attributes of parent class unavailable when inheriting.
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$data = <<<'NOWDOC'
<?php
class abc {
        private $arr;
        public function abc(){
                $this->arr = array(
                        0 => array("some", "values"),
                        1 => array("some", "more", "values")
                );
        }
}

class def extends abc {
        private $arr;
}

$o = new def;
echo "o: ";
var_dump($o);
NOWDOC;

$commands = array(
	'step_into',
	'breakpoint_set -t line -n 17',
	'run',
	'property_get -n o',
	'detach'
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///tmp/xdebug-dbgp-test.php" language="PHP" protocol_version="1.0" appid="%d" idekey="%s"><engine version="%s"><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2010 by Derick Rethans]]></copyright></init>

-> step_into -i 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="step_into" transaction_id="1" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="2"></xdebug:message></response>

-> breakpoint_set -i 2 -t line -n 17
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file:///tmp/xdebug-dbgp-test.php" lineno="17"></xdebug:message></response>

-> property_get -i 4 -n o
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="property_get" transaction_id="4"><property name="o" fullname="$o" address="" type="object" classname="def" children="1" numchildren="2" page="0" pagesize="32"><property name="CLASSNAME" type="string"><![CDATA[def]]></property><property name="arr" fullname="$o-&gt;arr" facet="private" address="" type="null"></property></property></response>

-> detach -i 5
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="detach" transaction_id="5" status="stopping" reason="ok"></response>
