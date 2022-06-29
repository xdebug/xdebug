--TEST--
Test for bug #2069: zlib, use_compression=1, append=1
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext-flag compression');
?>
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
xdebug.use_compression=1
xdebug.profiler_append=1
xdebug.profiler_output_name=cachegrind.out.%R.end
xdebug.log={TMP}/{RUNID}{TEST_PHP_WORKER}issue2069-001.txt
--FILE--
<?php
require_once 'capture-profile.inc';
$file = xdebug_get_profiler_filename();
var_dump( $file );

echo "\n", file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'issue2069-001.txt' );
@unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'issue2069-001.txt' );
?>
--EXPECTF--
string(%d) "%send"

[%d] Log opened at %s
[%d] [Config] WARN: Cannot append to profiling file while file compression is turned on. Falling back to creating an uncompressed file
%A
