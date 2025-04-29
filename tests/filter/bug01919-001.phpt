--TEST--
Test for bug #1919: Crash with XDEBUG_FILTER_CODE_COVERAGE without xdebug.mode=coverage
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('!win');
?>
--INI--
xdebug.mode=develop
xdebug.log={TMPFILE:issue1919-001.txt}
xdebug.control_socket=off
--FILE--
<?php
require_once __DIR__ . '/../utils.inc';

xdebug_set_filter(XDEBUG_FILTER_CODE_COVERAGE, XDEBUG_PATH_INCLUDE, []);
echo file_get_contents(getTmpFile('issue1919-001.txt'));
@unlink(getTmpFile('issue1919-001.txt'));
?>
--EXPECTF--
[%d] Log opened at %s
[%d] [Base] WARN: Can not set a filter for code coverage, because Xdebug mode does not include 'coverage'
