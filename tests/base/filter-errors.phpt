--TEST--
xdebug_set_filter() wrong arguments
--INI--
html_errors=0
display_errors=1
xdebug.mode=coverage
--FILE--
<?php
xdebug_set_filter(42, XDEBUG_PATH_EXCLUDE, [ "xdebug" ] );

xdebug_set_filter(XDEBUG_FILTER_CODE_COVERAGE, 42, [ "xdebug" ] );

xdebug_set_filter(XDEBUG_FILTER_CODE_COVERAGE, XDEBUG_NAMESPACE_EXCLUDE, [ "xdebug" ] );
xdebug_set_filter(XDEBUG_FILTER_CODE_COVERAGE, XDEBUG_NAMESPACE_INCLUDE, [ "xdebug" ] );
?>
--EXPECTF--
Warning: Filter group needs to be one of XDEBUG_FILTER_CODE_COVERAGE, XDEBUG_FILTER_STACK, or XDEBUG_FILTER_TRACING in %sfilter-errors.php on line 2

Warning: Filter type needs to be one of XDEBUG_PATH_INCLUDE, XDEBUG_PATH_EXCLUDE, XDEBUG_NAMESPACE_INCLUDE, XDEBUG_NAMESPACE_EXCLUDE, or XDEBUG_FILTER_NONE in %sfilter-errors.php on line 4

Warning: The code coverage filter (XDEBUG_FILTER_CODE_COVERAGE) only supports the XDEBUG_PATH_INCLUDE, XDEBUG_PATH_EXCLUDE, and XDEBUG_FILTER_NONE filter types in %sfilter-errors.php on line 6

Warning: The code coverage filter (XDEBUG_FILTER_CODE_COVERAGE) only supports the XDEBUG_PATH_INCLUDE, XDEBUG_PATH_EXCLUDE, and XDEBUG_FILTER_NONE filter types in %sfilter-errors.php on line 7
