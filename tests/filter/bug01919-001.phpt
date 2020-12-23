--TEST--
Test for bug #1919: Crash with XDEBUG_FILTER_CODE_COVERAGE without xdebug.mode=coverage
--INI--
xdebug.mode=develop
xdebug.log={TMPDIR}/{RUNID}issue1919-001.txt
--FILE--
<?php
xdebug_set_filter(XDEBUG_FILTER_CODE_COVERAGE, XDEBUG_PATH_INCLUDE, []);
echo file_get_contents(sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . 'issue1919-001.txt' );
@unlink (sys_get_temp_dir() . '/' . getenv('UNIQ_RUN_ID') . 'issue1919-001.txt' );
?>
--EXPECTF--
[%d] Log opened at %s
[%d] [Base] WARN: Can not set a filter for code coverage, because Xdebug mode does not include 'coverage'
