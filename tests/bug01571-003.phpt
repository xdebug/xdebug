--TEST--
Test for bug #1571: Code Coverage doesn't show file/line for closures in namespaces (PHP < 7.3 || (PHP >= 7.3 && !opcache))
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.overload_var_dump=0
--SKIPIF--
<?php
if (version_compare(phpversion(), "7.3", '>=') && extension_loaded('zend opcache') ) {
    echo "skip (PHP < 7.3 || (PHP >= 7.3 && !opcache)) needed\n";
}
?>
--FILE--
<?php
namespace Testing;

$pathname = stream_resolve_include_path(__FILE__);
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

$cb = function($a) {
	usleep(500);
	return $a * 3;
};
$cb(4);

$coverage = xdebug_get_code_coverage();
xdebug_stop_code_coverage();
print_r(array_keys($coverage[$pathname]['functions']));
?>
--EXPECTF--
Array
(
    [0] => Testing\{closure:%sbug01571-003.php:7-10}
)
