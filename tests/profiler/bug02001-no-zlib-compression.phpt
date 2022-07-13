--TEST--
Test for bug #2001: no zlib, use_compression=1
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!ext-flag compression');
?>
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
xdebug.use_compression=1
xdebug.profiler_output_name=cachegrind.out
xdebug.log={TMP}/{RUNID}{TEST_PHP_WORKER}bug2001-no-zlib-compression.txt
--FILE--
<?php
require_once 'capture-profile.inc';

echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'bug2001-no-zlib-compression.txt' );
unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'bug2001-no-zlib-compression.txt' );
?>
--EXPECTF--
[%d] Log opened at %s
[%d] [Config] WARN: Cannot create the compressed file '%s.out.gz', because support for zlib has not been compiled in. Falling back to '%s.out'
%A
