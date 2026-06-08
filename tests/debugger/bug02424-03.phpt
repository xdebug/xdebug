--TEST--
Test for bug #2424: Ctrl Socket: 'ps' command
--INI--
xdebug.mode=debug
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('linux');
?>
--FILE--
<?php
require 'dbgp/ctrlSocketClient.php';

$c = new CtrlSocketClient( 'bug02424-03' );

$payload = 'ps';

$c->runCommand($payload);
?>
--EXPECTF--
<?xml version="1.0" encoding="UTF-8"?>
<ctrl-response xmlns:xdebug-ctrl="https://xdebug.org/ctrl/xdebug"><ps success="1"><engine version=""><![CDATA[Xdebug]]></engine><fileuri></fileuri><pid></pid><time></time><memory></memory></ps></ctrl-response>
