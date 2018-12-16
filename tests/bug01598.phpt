--TEST--
Test for bug #1598: Disallow path/branch coverage when OPcache is loaded for PHP 7.3 and later
--SKIPIF--
<?php
if (!version_compare(phpversion(), "7.3", '>=')) echo "skip >= PHP 7.3 needed\n";
if (!extension_loaded('zend opcache')) echo "skip opcache required\n";
?>
--FILE--
<?php
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);
var_dump(xdebug_stop_code_coverage());
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);
var_dump(xdebug_stop_code_coverage());
?>
--EXPECTF--
bool(true)

Warning: You can not use path/branch coverage when Zend OPcache is also loaded. Please disable Zend OPcache in %sbug01598.php on line 4
bool(false)
