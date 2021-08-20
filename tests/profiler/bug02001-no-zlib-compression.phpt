--TEST--
Test for bug #2001: no zlib, use_compression=1
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!ext-flag compression; !win');
?>
--INI--
xdebug.mode=profile
xdebug.start_with_request=default
xdebug.use_compression=1
xdebug.profiler_output_name=cachegrind.out
xdebug.log={TMPDIR}/{RUNID}bug2001-no-zlib-compression.txt
--FILE--
<?php
$file = xdebug_get_profiler_filename();
var_dump($file);
if ($file) {
	unlink($file);
}

echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . 'bug2001-no-zlib-compression.txt' );
unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . 'bug2001-no-zlib-compression.txt' );
?>
--EXPECTF--
string(%d) "%scachegrind.out"
[%d] Log opened at %s
[%d] [Config] WARN: Cannot create the compressed file '%s.out.gz', because support for zlib has not been compiled in. Falling back to '%s.out'
