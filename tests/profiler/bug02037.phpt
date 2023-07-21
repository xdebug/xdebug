--TEST--
Test for bug #2037: Crash when profile file can not be created
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!win');
?>
--INI--
xdebug.mode=profile
xdebug.log={TMP}/{RUNID}{TEST_PHP_WORKER}issue2037.txt
xdebug.output_dir=/tmp/un-writable
--FILE--
<?php
echo "==DONE==\n";
echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'issue2037.txt' );
@unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'issue2037.txt' );
?>
--EXPECTF--
==DONE==
[%d] Log opened at %s
[%d] [Profiler] ERR: File '/tmp/un-writable/%s' could not be opened.
[%d] [Profiler] WARN: /tmp/un-writable: %s
