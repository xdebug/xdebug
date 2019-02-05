--TEST--
Test for bug #627: breakpoints set in symlinked files don't trigger
--SKIPIF--
<?php
if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); }
if (substr(PHP_OS, 0, 3) == "WIN") { exit("skip Not for Windows"); }
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
$dir = dirname(__FILE__);
putenv("XDEBUG_TEST_DIR=$dir");
$data = file_get_contents($dir . '/bug00627.inc');

$commands = array(
	"stdout -c 1",
	"breakpoint_set -t line -f file://$dir/bug00627-symlink.inc -n 4",
	"run",
	'context_get -d 0',
);

dbgpRun( $data, $commands );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///%sxdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-%d by Derick Rethans]]></copyright></init>

-> stdout -i 1 -c 1
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="stdout" transaction_id="1" success="1"></response>

-> breakpoint_set -i 2 -t line -f file:///%s/tests/bug00627-symlink.inc -n 4
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="breakpoint_set" transaction_id="2" id=""></response>

-> run -i 3
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="run" transaction_id="3" status="break" reason="ok"><xdebug:message filename="file://%sbug00627-symlink-target.inc" lineno="4"></xdebug:message></response>

-> context_get -i 4 -d 0
<?xml version="1.0" encoding="iso-8859-1"?>
<response xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" command="context_get" transaction_id="4" context="0"></response>
