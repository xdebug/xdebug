--TEST--
Test for bug #2424: Ctrl Socket: Command that doesn't exist
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

$c = new CtrlSocketClient( 'bug02424-02' );

$payload = '';
for ($i = 0; $i < 256; $i++) {
	$payload .= chr(mt_rand(0x41, 0x5A));
}

$c->runCommand($payload);

?>
--EXPECTF--
<?xml version="1.0" encoding="UTF-8"?>
<ctrl-response xmlns:xdebug-ctrl="https://xdebug.org/ctrl/xdebug"><error code="5"><message><![CDATA[command is not available]]></message></error></ctrl-response>
