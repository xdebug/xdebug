--TEST--
Test for bug #422: Segfaults when using code coverage with a parse error in the script
--INI--
xdebug.mode=coverage
--FILE--
<?php
function hdl(){}
set_error_handler('hdl');
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE);
try {
	require_once 'bug00422.inc';
} catch (ParseError $err) {
	echo $err->getMessage(), "\n";
}
echo "END";
?>
--EXPECTF--
syntax error, unexpected%Snew%S
END
