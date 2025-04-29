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
xdebug.log={TMPFILE:bug2001-no-zlib-compression.txt}
xdebug.control_socket=off
--FILE--
<?php
require __DIR__ . '/../utils.inc';
require_once 'capture-profile.inc';

echo file_get_contents(getTmpFile('bug2001-no-zlib-compression.txt'));
unlink(getTmpFile('bug2001-no-zlib-compression.txt'));
?>
--EXPECTF--
[%d] Log opened at %s
[%d] [Config] WARN: Cannot create the compressed file '%s.out.gz', because support for zlib has not been compiled in. Falling back to '%s.out'
%A
