--TEST--
Compression: no zlib, use_compression=1
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!ext-flag compression');
?>
--INI--
xdebug.mode=trace
xdebug.start_with_request=yes
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.use_compression=1
xdebug.log={TMP}/{RUNID}{TEST_PHP_WORKER}start_no_zlib_compression.txt
--FILE--
<?php
$tf = xdebug_get_tracefile_name();

xdebug_stop_trace();

echo $tf, "\n";
if (preg_match('@\.gz$@', $tf)) {
	$fp = gzopen($tf, 'r');
	echo stream_get_contents($fp);
} else {
	echo file_get_contents($tf);
}

echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'start_no_zlib_compression.txt' );
?>
--CLEAN--
<?php
unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . getenv('TEST_PHP_WORKER') . 'start_no_zlib_compression.txt' );
?>
--EXPECTF--
%s.xt
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d   -> {main}() %s:0
%w%f %w%d     -> xdebug_get_tracefile_name() %s:2
%w%f %w%d     -> xdebug_stop_trace() %s:4
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]

[%d] Log opened at %s
[%d] [Config] WARN: Cannot create the compressed file '%s.xt.gz', because support for zlib has not been compiled in. Falling back to '%s.xt'
