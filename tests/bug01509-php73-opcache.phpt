--TEST--
Test for bug #1509: Code coverage missing for case inside switch (>= PHP 7.3, opcache)
--SKIPIF--
<?php
if (version_compare(phpversion(), "7.3", '<')) echo "skip >= PHP 7.2, < PHP 7.3 needed\n";
if (!extension_loaded('zend opcache')) echo "skip opcache required\n";
?>
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.overload_var_dump=0
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
    [2] => 1
    [6] => 1
    [7] => 1
    [8] => 1
    [9] => -2
    [10] => 1
    [11] => 1
    [12] => -2
    [14] => 1
    [15] => -2
    [17] => -2
    [20] => 1
    [21] => 1
    [22] => 1
    [25] => 1
)
