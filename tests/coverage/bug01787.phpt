--TEST--
Test for bug #1787: Branch coverage data does not always follow the lines/functions format
--INI--
xdebug.mode=coverage
xdebug.start_with_request=trigger
--FILE--
<?php
xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

require dirname(__FILE__) . '/bug01787.inc';

$coverage = xdebug_get_code_coverage();
xdebug_stop_code_coverage();

ksort( $coverage );

foreach ( $coverage as $file => $info ) {
	echo $file, isset( $info['functions'] ) ? ' has ' : ' does not have ', "'functions' array\n";
	echo $file, isset( $info['lines'] ) ? ' has ' : ' does not have ', "'lines' array\n";
}
?>
--EXPECTF--
%sbug01787.inc has 'functions' array
%sbug01787.inc has 'lines' array
%sbug01787.php has 'functions' array
%sbug01787.php has 'lines' array
