--TEST--
Test for bug #2267: Application slow when debugger is not running VS Code (Unix)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('dbgp; !win');
if (getenv('CIRCLECI') !== false) die("skip on CircleCI");
?>
--INI--
xdebug.mode=debug
xdebug.start_with_request=no
xdebug.log={TMPFILE:bug02267.txt}
xdebug.log_level=12
xdebug.client_host=localhost
xdebug.client_port=9119
xdebug.control_socket=off
xdebug.path_mapping=off
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

require __DIR__ . '/bug02267.inc';

echo file_get_contents(getTmpFile('bug02267.txt'));
?>
--CLEAN--
<?php
require_once __DIR__ . '/../utils.inc';
@unlink(getTmpFile('bug02267.txt'));
?>
--EXPECTF--
Before: %f
After:  %f
Diff:   0.00%d

[%d] Log opened at %d-%d-%d %d:%d:%d.%d
[%d] [Step Debug] INFO: Connecting to configured address/port: localhost:9119.
[%d] [Step Debug] INFO: Connecting to localhost: yes
[%d] [Step Debug] WARN: Creating socket for 'localhost:9119', poll success, but error: %s
[%d] [Step Debug] INFO: Connecting to localhost: yes
[%d] [Step Debug] WARN: Creating socket for 'localhost:9119', poll success, but error: %s
[%d] [Step Debug] ERR: Could not connect to debugging client. Tried: localhost:9119 (through xdebug.client_host/xdebug.client_port).
