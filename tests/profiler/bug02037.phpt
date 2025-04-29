--TEST--
Test for bug #2037: Crash when profile file can not be created
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!win');
?>
--INI--
xdebug.mode=profile
xdebug.log={TMPFILE:issue2037.txt}
xdebug.output_dir=/tmp/un-writable
xdebug.control_socket=off
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

echo "==DONE==\n";
echo file_get_contents(getTmpFile('issue2037.txt'));
@unlink(getTmpFile('issue2037.txt'));
?>
--EXPECTF--
==DONE==
[%d] Log opened at %s
[%d] [Profiler] ERR: File '/tmp/un-writable/%s' could not be opened.
[%d] [Profiler] WARN: /tmp/un-writable: %s
