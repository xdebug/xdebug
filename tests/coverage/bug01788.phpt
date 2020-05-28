--TEST--
Test for bug #1788: Branch coverage data does not always follow the lines/functions format
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.overload_var_dump=0
xdebug.coverage_enable=1
--FILE--
<?php
require dirname(__FILE__) . '/bug01788a.inc';
require dirname(__FILE__) . '/bug01788b.inc';

xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

$bar = 'bar';

$class = new Baz();
$class->bar();

$coverage = xdebug_get_code_coverage();
xdebug_stop_code_coverage();

ksort( $coverage );

foreach ( $coverage as $file => $info ) {
	echo $file, isset( $info['functions'] ) ? ' has ' : ' does not have ', "'functions' array\n";
	echo $file, isset( $info['lines'] ) ? ' has ' : ' does not have ', "'lines' array\n";
}
?>
--EXPECTF--
%sbug01788.php has 'functions' array
%sbug01788.php has 'lines' array
%sbug01788b.inc has 'functions' array
%sbug01788b.inc has 'lines' array
