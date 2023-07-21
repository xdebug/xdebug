--TEST--
Test for bug #1508: Code coverage filter not checked in xdebug_common_assign_dim handler (white list)
--INI--
xdebug.mode=coverage
--FILE--
<?php
$cwd = __DIR__; $s = DIRECTORY_SEPARATOR;
xdebug_set_filter(XDEBUG_FILTER_CODE_COVERAGE, XDEBUG_PATH_INCLUDE, [] );

$tf = xdebug_start_code_coverage( XDEBUG_CC_DEAD_CODE | XDEBUG_CC_UNUSED );

$file = "{$cwd}/bug01508.php";
include "{$file}";

Filter::bug1508();
	
$result = xdebug_get_code_coverage();

echo array_key_exists( $file, $result ) ? "File '{$file}' is present in code coverage" : "File is not present in code coverage", "\n";
?>
--EXPECTF--
File is not present in code coverage
