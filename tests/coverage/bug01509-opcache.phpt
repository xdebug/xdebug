--TEST--
Test for bug #1509: Code coverage missing for case inside switch (opcache)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('opcache');
?>
--INI--
xdebug.mode=coverage
xdebug.auto_profile=0
--FILE--
<?php
$file = 'bug01509.inc';
$pathname = stream_resolve_include_path($file);
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);
require $pathname;
$coverage = xdebug_get_code_coverage();
xdebug_stop_code_coverage();
print_r($coverage[$pathname]);
?>
--EXPECTF--
Array
(
    [6] => 1
    [7] => 1
    [8] => 1
    [10] => 1
    [11] => 1
    [14] => 1
    [20] => 1
    [21] => 1
    [22] => 1
    [25] => 1
)
