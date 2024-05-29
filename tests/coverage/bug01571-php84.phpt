--TEST--
Test for bug #1571: Code Coverage doesn't show file/line for closures in namespaces (>= PHP 8.4)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.4');
?>
--INI--
xdebug.mode=coverage
xdebug.auto_profile=0
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
    [0] => {closure:%sbug01571-php84.php:7-10}
)
