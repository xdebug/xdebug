--TEST--
Test for bug #2085: Crash when used with source guardian encoded files
--INI--
xdebug.mode=coverage
--FILE--
<?php
xdebug_set_filter(
	XDEBUG_FILTER_CODE_COVERAGE,
	XDEBUG_PATH_INCLUDE,
	["src/"]
);
xdebug_start_code_coverage();
require_once __DIR__ . "/bug02085.inc";
xdebug_stop_code_coverage();

?>
--EXPECTF--
%AI'm encoded
