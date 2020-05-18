--TEST--
Test for bug #932: Show an error if Xdebug can't open the remote debug log
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; !win');
?>
--INI--
xdebug.mode=debug
xdebug.start_with_request=never
error_log=
xdebug.remote_autostart=1
xdebug.remote_log=/tmp/{RUNID}bug932.log
xdebug.remote_port=9999
xdebug.force_error_reporting=0
--FILE--
<?php
$file = '/tmp/' . getenv('UNIQ_RUN_ID') . 'bug932.log';
touch($file);
chmod($file, 0);

@trigger_error('foo');

unlink($file);
?>
--EXPECTF--
Xdebug could not open the remote debug file '/tmp/%Sbug932.log'.
