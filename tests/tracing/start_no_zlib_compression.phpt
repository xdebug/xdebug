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
xdebug.trace_format=0
xdebug.trace_output_name=trace.%p.%r
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.use_compression=1
xdebug.log={TMPFILE:start_no_zlib_compression.txt}
xdebug.control_socket=off
--FILE--
<?php
require __DIR__ . '/../utils.inc';

$tf = xdebug_get_tracefile_name();

xdebug_stop_trace();

echo $tf, "\n";
if (preg_match('@\.gz$@', $tf)) {
	$fp = gzopen($tf, 'r');
	echo stream_get_contents($fp);
} else {
	echo file_get_contents($tf);
}

echo file_get_contents(getTmpFile('start_no_zlib_compression.txt'));
?>
--CLEAN--
<?php
require __DIR__ . '/../utils.inc';

unlink(getTmpFile('start_no_zlib_compression.txt'));
?>
--EXPECTF--
%s.xt
TRACE START [%d-%d-%d %d:%d:%d.%d]
%w%f %w%d   -> {main}() %s:0
%w%f %w%d     -> require(%s) %s:2
%w%f %w%d     -> xdebug_get_tracefile_name() %s:4
%w%f %w%d     -> xdebug_stop_trace() %s:6
%w%f %w%d
TRACE END   [%d-%d-%d %d:%d:%d.%d]

[%d] Log opened at %s
[%d] [Config] WARN: Cannot create the compressed file '%s.xt.gz', because support for zlib has not been compiled in. Falling back to '%s.xt'
