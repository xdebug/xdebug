--TEST--
Test for bug #998: Test that Xdebug connects back on IPv6 with localhost as the remote host
--SKIPIF--
<?php
if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); }
require 'dbgp/dbgpclient.php';
if (!DebugClientIPv6::isSupported()) echo "skip IPv6 support is not configured.\n";
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
dbgpRun("", array(), array("xdebug.remote_host" => "localhost"), XDEBUG_DBGP_IPV6);
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="http://xdebug.org/dbgp/xdebug" fileuri="file:///tmp/xdebug-dbgp-test.php" language="PHP" xdebug:language_version="" protocol_version="1.0" appid="" idekey=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[http://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-%d by Derick Rethans]]></copyright></init>
