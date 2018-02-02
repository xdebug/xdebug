--TEST--
Test for bug #1508: Code coverage filter not checked in xdebug_common_assign_dim handler (white list)
--INI--
xdebug.auto_trace=0
xdebug.collect_return=1
xdebug.collect_params=4
xdebug.collect_assignments=0
xdebug.trace_format=0
--FILE--
<?php
$cwd = __DIR__; $s = DIRECTORY_SEPARATOR;
xdebug_set_filter(XDEBUG_FILTER_CODE_COVERAGE, XDEBUG_PATH_WHITELIST, [] );

$tf = xdebug_start_code_coverage( XDEBUG_CC_DEAD_CODE | XDEBUG_CC_UNUSED );

$file = "{$cwd}/filter/bug01508.php";
include "{$file}";

Filter::bug1508();
	
$result = xdebug_get_code_coverage();

echo array_key_exists( $file, $result ) ? "File '{$file}' is present in code coverage" : "File is not present in code coverage", "\n";
?>
--EXPECTF--
File is not present in code coverage
