--TEST--
Test for bug #998: Test that Xdebug connects back on IPv6
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; !win');
if (getenv("SKIP_IPV6_TESTS")) { exit("skip Excluding IPv6 tests"); }
require 'dbgp/dbgpclient.php';
if (!DebugClientIPv6::isSupported($errno, $errstr)) echo "skip IPv6 support is not configured. Error: $errstr, errno: $errno\n";
?>
--FILE--
<?php
require 'dbgp/dbgpclient.php';
dbgpRunFile('', [], [], [ 'ipv' => 6 ] );
?>
--EXPECTF--
<?xml version="1.0" encoding="iso-8859-1"?>
<init xmlns="urn:debugger_protocol_v1" xmlns:xdebug="https://xdebug.org/dbgp/xdebug" fileuri="file://" language="PHP" xdebug:language_version="" protocol_version="1.0" appid=""><engine version=""><![CDATA[Xdebug]]></engine><author><![CDATA[Derick Rethans]]></author><url><![CDATA[https://xdebug.org]]></url><copyright><![CDATA[Copyright (c) 2002-2099 by Derick Rethans]]></copyright></init>
