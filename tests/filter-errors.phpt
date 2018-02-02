--TEST--
xdebug_set_filter() wrong arguments
--INI--
html_errors=0
display_errors=1
--FILE--
<?php
xdebug_set_filter(42, XDEBUG_PATH_BLACKLIST, [ "xdebug" ] );

xdebug_set_filter(XDEBUG_FILTER_CODE_COVERAGE, 42, [ "xdebug" ] );

xdebug_set_filter(XDEBUG_FILTER_CODE_COVERAGE, XDEBUG_NAMESPACE_BLACKLIST, [ "xdebug" ] );
xdebug_set_filter(XDEBUG_FILTER_CODE_COVERAGE, XDEBUG_NAMESPACE_WHITELIST, [ "xdebug" ] );
?>
--EXPECTF--
Warning: Filter group needs to be one of XDEBUG_FILTER_TRACING or XDEBUG_FILTER_CODE_COVERAGE in %sfilter-errors.php on line 2

Warning: Filter type needs to be one of XDEBUG_PATH_WHITELIST, XDEBUG_PATH_BLACKLIST, XDEBUG_NAMESPACE_WHITELIST, XDEBUG_NAMESPACE_BLACKLIST, or XDEBUG_FILTER_NONE in %sfilter-errors.php on line 4

Warning: The code coverage filter (XDEBUG_FILTER_CODE_COVERAGE) only supports the XDEBUG_PATH_WHITELIST, XDEBUG_PATH_BLACKLIST, and XDEBUG_FILTER_NONE filter types in %sfilter-errors.php on line 6

Warning: The code coverage filter (XDEBUG_FILTER_CODE_COVERAGE) only supports the XDEBUG_PATH_WHITELIST, XDEBUG_PATH_BLACKLIST, and XDEBUG_FILTER_NONE filter types in %sfilter-errors.php on line 7
