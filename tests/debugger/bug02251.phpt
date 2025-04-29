--TEST--
Test for bug #2251: xdebug.log setting not picked up from XDEBUG_CONFIG
--ENV--
XDEBUG_CONFIG=log={TMPFILE:bug02251.log}
--INI--
xdebug.mode=debug,develop
default_charset=utf-8
xdebug.filename_format=
xdebug.client_port=9172
xdebug.start_with_request=yes
xdebug.log=
xdebug.log_level=10
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

echo file_get_contents(getTmpFile('bug02251.log'));
@unlink(getTmpFile('bug02251.log'));
?>
--EXPECTF--
%A[Step Debug] %sTried: localhost:9172 (through xdebug.client_host/xdebug.client_port).
