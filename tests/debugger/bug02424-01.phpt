--TEST--
Test for bug #2424: Ctrl Socket: Aliveness after empty command
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

$c1 = new CtrlSocketClient( 'bug02424-01a' );
$c1->runCommand('');

$c2 = new CtrlSocketClient( 'bug02424-01b' );
$c2->runCommand('ps');
?>
--EXPECTF--
<?xml version="1.0" encoding="UTF-8"?>
<ctrl-response xmlns:xdebug-ctrl="https://xdebug.org/ctrl/xdebug"><ps success="1"><engine version=""><![CDATA[Xdebug]]></engine><fileuri></fileuri><pid></pid><time></time><memory></memory></ps></ctrl-response>
